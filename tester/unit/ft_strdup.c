#define _GNU_SOURCE
#include "../include/unit.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

// Build with -Wl,--wrap=malloc: every call to malloc made by the test and by
// ft_strdup goes through __wrap_malloc below, which records the call and can
// be told to fail. __real_malloc is the libc malloc.
extern char *ft_strdup(const char *s);
void *__real_malloc(size_t size);

#define BIG_LEN (((size_t)1 << 20) + 3)

// Sentinel errno value: lets us see whether errno was touched.
#define ERRNO_SENTINEL 4242

// Byte malloc fills its blocks with: a copy that forgets a byte (or the
// terminating nul) leaves it visible instead of a lucky 0.
#define POISON 0xab

static size_t g_calls;
static size_t g_last_size;
static bool g_fail;

void *__wrap_malloc(size_t size)
{
	g_calls++;
	g_last_size = size;
	if (g_fail)
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

// Runs a test in a child process, so a crash (segfault...) is reported as a
// failure of that test instead of killing every test after it.
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

// Maps 3 pages, makes the first and last PROT_NONE and returns the middle one:
// any access outside the middle page segfaults.
static char *guarded_page(size_t *page)
{
	*page = (size_t)sysconf(_SC_PAGESIZE);
	char *map = mmap(NULL, *page * 3, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (map == MAP_FAILED)
		return NULL;
	if (mprotect(map, *page, PROT_NONE) ||
		mprotect(map + 2 * *page, *page, PROT_NONE))
	{
		munmap(map, *page * 3);
		return NULL;
	}
	return map + *page;
}

static void release_page(char *mid, size_t page)
{
	munmap(mid - page, page * 3);
}

// Non-zero, position-dependent pattern with high bytes (never a nul).
static void fill_pattern(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		buf[i] = (char)(i * 7 + 3);
		if (buf[i] == 0)
			buf[i] = 1;
	}
}

// Core check: ft_strdup(src) asks malloc for exactly strlen(src) + 1 bytes,
// once, returns a new pointer (not src), the copy holds the same bytes and
// its nul, src is left untouched, and the copy can be written and freed.
static bool dup_ok(const char *src)
{
	const size_t len = strlen(src);
	char *before = __real_malloc(len + 1);
	bool ok = before != NULL;

	if (ok)
	{
		memcpy(before, src, len + 1);
		spy_reset();
		char *d = ft_strdup(src);
		const size_t calls = g_calls, asked = g_last_size;
		ok = d && d != src && calls == 1 && asked == len + 1 &&
			 memcmp(d, src, len + 1) == 0 && memcmp(src, before, len + 1) == 0;
		if (!ok)
			fprintf(stderr,
					"  len=%zu: ret=%p src=%p malloc calls=%zu size=%zu"
					" (want 1 call of %zu)\n",
					len, (void *)d, (void *)src, calls, asked, len + 1);
		if (d)
		{
			memset(d, 'z', len);
			ok = ok && memcmp(src, before, len + 1) == 0;
			free(d);
		}
	}
	free(before);
	return ok;
}

static bool dup_classic(void) { return dup_ok("Hello World!"); }

static bool dup_one_char(void) { return dup_ok("a"); }

// Empty string: a 1-byte block holding only the nul, never NULL.
static bool dup_empty(void) { return dup_ok(""); }

// Every non-nul byte value, including the high ones (signed char < 0).
static bool dup_all_bytes(void)
{
	char s[256];

	for (int i = 1; i < 256; i++)
		s[i - 1] = (char)i;
	s[255] = '\0';
	return dup_ok(s);
}

// The copy stops at the first nul: bytes after it are not copied and do not
// count in the size.
static bool dup_embedded_nul(void)
{
	static const char s[] = "abc\0def";
	spy_reset();
	char *d = ft_strdup(s);
	bool ok = d && g_last_size == 4 && memcmp(d, "abc", 4) == 0;

	free(d);
	return ok;
}

// Two calls on the same string give two different blocks.
static bool dup_distinct(void)
{
	char *a = ft_strdup("same");
	char *b = ft_strdup("same");
	bool ok = a && b && a != b && strcmp(a, b) == 0;

	free(a);
	free(b);
	return ok;
}

// malloc fails: NULL comes back (all 64 bits), nothing is written through the
// NULL pointer (that would crash), and errno is the ENOMEM malloc set.
static bool dup_malloc_fails(void)
{
	spy_reset();
	g_fail = true;
	errno = ERRNO_SENTINEL;
	char *d = ft_strdup("will not fit");
	const int e = errno;
	const size_t calls = g_calls;

	g_fail = false;
	if (d != NULL || e != ENOMEM || calls != 1)
	{
		fprintf(stderr, "  got ret=%p errno=%d calls=%zu, want NULL/%d/1\n",
				(void *)d, e, calls, ENOMEM);
		return false;
	}
	return true;
}

// Same content as the libc strdup.
static bool dup_same_as_libc(void)
{
	static const char *const cases[] = {"", "x", "Hello World!",
										"\x80\xff\x01 tab\t nl\n"};
	bool ok = true;

	for (size_t i = 0; i < sizeof(cases) / sizeof(*cases) && ok; i++)
	{
		char *ft = ft_strdup(cases[i]);
		char *lc = strdup(cases[i]);
		ok = ft && lc && strcmp(ft, lc) == 0;
		free(ft);
		free(lc);
	}
	return ok;
}

// Every length in [0, 64] at every source misalignment in [0, 15].
static bool dup_sweep(void)
{
	static char buf[128] __attribute__((aligned(64)));

	for (size_t len = 0; len <= 64; len++)
		for (size_t off = 0; off < 16; off++)
		{
			fill_pattern(buf, sizeof(buf));
			buf[off + len] = '\0';
			if (!dup_ok(buf + off))
			{
				fprintf(stderr, "  first mismatch: len=%zu off=%zu\n", len,
						off);
				return false;
			}
		}
	return true;
}

// Lengths around the 8/16-bit boundaries and past 65535: catches a length
// kept in a sub-register.
static bool dup_big(void)
{
	static const size_t lens[] = {255, 256, 257, 65535, 65536, 65537, BIG_LEN};
	char *s = __real_malloc(BIG_LEN + 1);
	bool ok = s != NULL;

	if (ok)
		fill_pattern(s, BIG_LEN);
	for (size_t i = 0; i < sizeof(lens) / sizeof(*lens) && ok; i++)
	{
		s[lens[i]] = '\0';
		ok = dup_ok(s);
		s[lens[i]] = 'x';
	}
	free(s);
	return ok;
}

// Source string whose nul is the last byte of a page: reading one byte past
// the nul (or starting before s) segfaults.
static bool dup_page_guard(void)
{
	size_t page;
	char *mid = guarded_page(&page);
	bool ok = mid != NULL;

	for (size_t len = 0; len <= 64 && ok; len++)
	{
		char *s = mid + page - len - 1;
		fill_pattern(s, len);
		s[len] = '\0';
		ok = dup_ok(s);
	}
	if (mid)
		release_page(mid, page);
	return ok;
}

int main(void)
{
	ASSERT("ft_strdup classic", isolated(dup_classic));
	ASSERT("ft_strdup one char", isolated(dup_one_char));
	ASSERT("ft_strdup empty string", isolated(dup_empty));
	ASSERT("ft_strdup all byte values", isolated(dup_all_bytes));
	ASSERT("ft_strdup stops at first nul", isolated(dup_embedded_nul));
	ASSERT("ft_strdup new block each call", isolated(dup_distinct));
	ASSERT("ft_strdup malloc fails", isolated(dup_malloc_fails));
	ASSERT("ft_strdup same as libc", isolated(dup_same_as_libc));
	ASSERT("ft_strdup length/alignment sweep", isolated(dup_sweep));
	ASSERT("ft_strdup long, length width", isolated(dup_big));
	ASSERT("ft_strdup no read past the nul", isolated(dup_page_guard));
}
