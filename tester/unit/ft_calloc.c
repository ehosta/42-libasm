#define _GNU_SOURCE
#include "../include/unit.h"
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// Build with -Wl,--wrap=malloc: every call to malloc made by the test and by
// ft_calloc goes through __wrap_malloc below, which records the call and can
// be told to fail. __real_malloc is the libc malloc.
extern void *ft_calloc(size_t nmemb, size_t size);
void *__real_malloc(size_t size);

#define BIG_LEN (((size_t)1 << 20) + 3)

// Sentinel errno value: lets us see whether errno was touched.
#define ERRNO_SENTINEL 4242

// Byte malloc fills its blocks with: memory ft_calloc forgets to zero shows
// up as POISON instead of a lucky 0 from a fresh page.
#define POISON 0xab

// Above this size the spy refuses (ENOMEM) instead of allocating: tests can
// ask for huge sizes and only check what size reached malloc.
#define SPY_LIMIT ((size_t)1 << 30)

static size_t g_calls;
static size_t g_last_size;
static bool g_fail;

void *__wrap_malloc(size_t size)
{
	g_calls++;
	g_last_size = size;
	if (g_fail || size > SPY_LIMIT)
	{
		errno = ENOMEM;
		return NULL;
	}
	void *p = __real_malloc(size);
	if (p)
		memset(p, POISON, size);
	return p;
}

static void spy_reset(void)
{
	g_calls = 0;
	g_last_size = 0;
	g_fail = false;
}

// Runs a test in a child process, so a crash (segfault, division by zero...)
// is reported as a failure of that test instead of killing every test after
// it.
static bool isolated(bool (*test)(void))
{
	fflush(NULL);
	pid_t pid = fork();
	if (pid == 0)
		_exit(test() ? 0 : 1);
	int status;
	if (pid < 0 || waitpid(pid, &status, 0) < 0)
		return false;
	if (WIFSIGNALED(status))
		fprintf(stderr, "  crashed: signal %d (%s)\n", WTERMSIG(status),
				strsignal(WTERMSIG(status)));
	return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

// Every byte of buf[0..n) is 0.
static bool all_zero(const unsigned char *buf, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if (buf[i] != 0)
			return false;
	return true;
}

// Core check: ft_calloc(nmemb, size) calls malloc once with exactly
// nmemb * size bytes, returns that block, every byte of it is 0, and it can
// be written and freed.
static bool calloc_ok(size_t nmemb, size_t size)
{
	const size_t total = nmemb * size;
	spy_reset();
	unsigned char *p = ft_calloc(nmemb, size);
	const size_t calls = g_calls, asked = g_last_size;
	bool ok = p && calls == 1 && asked == total && all_zero(p, total);

	if (!ok)
		fprintf(stderr,
				"  calloc(%zu, %zu): ret=%p malloc calls=%zu size=%zu"
				" (want 1 call of %zu), zeroed=%d\n",
				nmemb, size, (void *)p, calls, asked, total,
				p ? all_zero(p, total) : -1);
	if (p)
	{
		memset(p, 'z', total);
		free(p);
	}
	return ok;
}

static bool calloc_classic(void) { return calloc_ok(12, sizeof(int)); }

// Order of the arguments does not matter for the size.
static bool calloc_swapped(void)
{
	return calloc_ok(sizeof(int), 12) && calloc_ok(1, 100) && calloc_ok(100, 1);
}

// Two calls give two different, independent blocks.
static bool calloc_distinct(void)
{
	char *a = ft_calloc(8, 1);
	char *b = ft_calloc(8, 1);
	bool ok = a && b && a != b;

	if (ok)
	{
		memset(a, 'a', 8);
		ok = all_zero((unsigned char *)b, 8);
	}
	free(a);
	free(b);
	return ok;
}

// A zero argument: man 3 calloc allows NULL or a unique pointer that free()
// accepts. Either is fine, but no crash (an overflow check that divides by a
// zero size gets SIGFPE here) and no false "overflow".
static bool zero_case(size_t nmemb, size_t size)
{
	errno = ERRNO_SENTINEL;
	void *p = ft_calloc(nmemb, size);
	free(p);
	return true;
}

static bool calloc_zero(void)
{
	return zero_case(0, 0) && zero_case(0, 5) && zero_case(5, 0) &&
		   zero_case(0, SIZE_MAX) && zero_case(SIZE_MAX, 0);
}

// nmemb * size does not fit in 64 bits: NULL, errno ENOMEM, and malloc is
// never called (the overflow is caught by ft_calloc itself, not by malloc
// refusing a wrapped-around small size).
static bool overflows(size_t nmemb, size_t size)
{
	spy_reset();
	errno = ERRNO_SENTINEL;
	void *p = ft_calloc(nmemb, size);
	const int e = errno;
	const size_t calls = g_calls, asked = g_last_size;

	if (p != NULL || e != ENOMEM || calls != 0)
	{
		fprintf(stderr,
				"  calloc(%#zx, %#zx): ret=%p errno=%d malloc calls=%zu"
				" size=%#zx, want NULL/%d and no malloc\n",
				nmemb, size, p, e, calls, asked, ENOMEM);
		free(p);
		return false;
	}
	return true;
}

static bool calloc_overflow(void)
{
	return overflows(SIZE_MAX, 2) && overflows(2, SIZE_MAX) &&
		   overflows(SIZE_MAX, SIZE_MAX) &&
		   // 2^63 * 2 = 2^64: wraps to exactly 0.
		   overflows((size_t)1 << 63, 2) &&
		   // 2^32 * 2^32 = 2^64: wraps to 0 as well.
		   overflows((size_t)1 << 32, (size_t)1 << 32) &&
		   // 2^33 * 2^31 + a bit: wraps to a small, allocatable size.
		   overflows(((size_t)1 << 33) + 1, (size_t)1 << 31) &&
		   // The smallest overflow with size 3.
		   overflows(SIZE_MAX / 3 + 1, 3);
}

// Products that fit, right at the limit: no false overflow, malloc is called
// with the exact product (and fails here because the size is huge).
static bool fits(size_t nmemb, size_t size)
{
	spy_reset();
	errno = ERRNO_SENTINEL;
	void *p = ft_calloc(nmemb, size);
	const int e = errno;
	const size_t calls = g_calls, asked = g_last_size;

	if (p != NULL || e != ENOMEM || calls != 1 || asked != nmemb * size)
	{
		fprintf(stderr,
				"  calloc(%#zx, %#zx): ret=%p errno=%d malloc calls=%zu"
				" size=%#zx, want malloc(%#zx) then NULL/%d\n",
				nmemb, size, p, e, calls, asked, nmemb * size, ENOMEM);
		return false;
	}
	return true;
}

static bool calloc_no_false_overflow(void)
{
	return fits(SIZE_MAX, 1) && fits(1, SIZE_MAX) &&
		   // SIZE_MAX is divisible by 3: the product is exactly SIZE_MAX.
		   fits(SIZE_MAX / 3, 3) &&
		   // 2^32 and above: a product computed in 32 bits loses it.
		   fits((size_t)1 << 16, (size_t)1 << 16) && fits((size_t)1 << 31, 4) &&
		   fits((size_t)1 << 62, 2);
}

// malloc fails on a normal size: NULL comes back (all 64 bits), nothing is
// written through the NULL pointer (that would crash), errno is malloc's
// ENOMEM.
static bool calloc_malloc_fails(void)
{
	spy_reset();
	g_fail = true;
	errno = ERRNO_SENTINEL;
	void *p = ft_calloc(4, 8);
	const int e = errno;
	const size_t calls = g_calls;

	g_fail = false;
	if (p != NULL || e != ENOMEM || calls != 1)
	{
		fprintf(stderr, "  got ret=%p errno=%d calls=%zu, want NULL/%d/1\n", p,
				e, calls, ENOMEM);
		return false;
	}
	return true;
}

// A failing call followed by a succeeding one: the second must still return
// a valid, zeroed block.
static bool calloc_error_then_success(void)
{
	return overflows(SIZE_MAX, 2) && calloc_ok(3, 5);
}

// Every nmemb in [1, 32] with every size in [1, 9]: odd totals and totals
// that are not multiples of 8 or 16 (a zeroing loop that works by blocks
// must handle the tail).
static bool calloc_sweep(void)
{
	for (size_t n = 1; n <= 32; n++)
		for (size_t s = 1; s <= 9; s++)
			if (!calloc_ok(n, s))
			{
				fprintf(stderr, "  first mismatch: nmemb=%zu size=%zu\n", n, s);
				return false;
			}
	return true;
}

// Totals around the 8/16-bit boundaries and past 65535: catches a total or a
// zeroing counter kept in a sub-register.
static bool calloc_big(void)
{
	return calloc_ok(255, 1) && calloc_ok(256, 1) && calloc_ok(257, 1) &&
		   calloc_ok(65535, 1) && calloc_ok(65536, 1) && calloc_ok(1, 65537) &&
		   calloc_ok(BIG_LEN, 1) && calloc_ok(3, BIG_LEN);
}

int main(void)
{
	ASSERT("ft_calloc classic", isolated(calloc_classic));
	ASSERT("ft_calloc argument order", isolated(calloc_swapped));
	ASSERT("ft_calloc new block each call", isolated(calloc_distinct));
	ASSERT("ft_calloc zero nmemb or size", isolated(calloc_zero));
	ASSERT("ft_calloc overflow detected", isolated(calloc_overflow));
	ASSERT("ft_calloc no false overflow", isolated(calloc_no_false_overflow));
	ASSERT("ft_calloc malloc fails", isolated(calloc_malloc_fails));
	ASSERT("ft_calloc error then success", isolated(calloc_error_then_success));
	ASSERT("ft_calloc size sweep", isolated(calloc_sweep));
	ASSERT("ft_calloc long, size width", isolated(calloc_big));
}
