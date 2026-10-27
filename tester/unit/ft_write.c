#define _GNU_SOURCE
#include "../include/unit.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern ssize_t ft_write(int fd, const void *buf, size_t count);

#define BIG_LEN (((size_t)1 << 20) + 3)

// Sentinel errno value: lets us see whether ft_write touched errno at all.
#define ERRNO_SENTINEL 4242

// Maps 3 pages, makes the first and last PROT_NONE and returns the middle one:
// any access outside the middle page segfaults (or EFAULTs in the kernel).
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

// Non-zero, position-dependent pattern with high bytes and embedded nuls
// (write must not care about string semantics).
static void fill_pattern(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
		buf[i] = (char)(i * 7 + 3);
}

// Reads exactly n bytes from fd into out. Returns false on short read.
static bool read_all(int fd, char *out, size_t n)
{
	size_t got = 0;

	while (got < n)
	{
		ssize_t r = read(fd, out + got, n - got);
		if (r <= 0)
			return false;
		got += (size_t)r;
	}
	return true;
}

// Nothing more is waiting on a (non-blocking) read end.
static bool pipe_empty(int fd)
{
	char c;
	int flags = fcntl(fd, F_GETFL);

	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	ssize_t r = read(fd, &c, 1);
	fcntl(fd, F_SETFL, flags);
	return r == -1 && errno == EAGAIN;
}

// Writes buf[0..n) through a pipe and checks: return value is exactly n
// (as a full 64-bit ssize_t), the reader gets exactly those bytes and
// nothing more, and buf is left untouched.
static bool write_pipe_ok(const char *buf, size_t n)
{
	int p[2];
	char *copy = malloc(n + 1);
	char *got = malloc(n + 1);
	bool ok = copy && got && pipe(p) == 0;

	if (ok)
	{
		memcpy(copy, buf, n);
		ok = ft_write(p[1], buf, n) == (ssize_t)n && read_all(p[0], got, n) &&
			 memcmp(got, buf, n) == 0 && pipe_empty(p[0]) &&
			 memcmp(buf, copy, n) == 0;
		close(p[0]);
		close(p[1]);
	}
	free(copy);
	free(got);
	return ok;
}

// ft_write and write agree on both the return value and errno.
// errno is reset to a sentinel before each call, so a missing or wrong
// errno write shows up.
static bool same_as_libc(int fd, const void *buf, size_t n)
{
	errno = ERRNO_SENTINEL;
	const ssize_t ft = ft_write(fd, buf, n);
	const int ft_err = errno;
	errno = ERRNO_SENTINEL;
	const ssize_t lc = write(fd, buf, n);
	const int lc_err = errno;

	if (ft != lc || (lc == -1 && ft_err != lc_err))
	{
		fprintf(stderr, "  fd=%d n=%zu: ft=%zd errno=%d | libc=%zd errno=%d\n",
				fd, n, ft, ft_err, lc, lc_err);
		return false;
	}
	return true;
}

// Failure must return exactly -1 (all 64 bits) and set errno to the positive
// error code. Catches: errno = rax without neg (-9), returning the raw
// -errno, returning -1 in eax only (rax = 0xffffffff).
static bool fails_with(int fd, const void *buf, size_t n, int err)
{
	errno = ERRNO_SENTINEL;
	const ssize_t r = ft_write(fd, buf, n);
	const int e = errno;

	if (r != -1 || e != err)
	{
		fprintf(stderr, "  fd=%d n=%zu: got ret=%zd errno=%d, want -1/%d\n", fd,
				n, r, e, err);
		return false;
	}
	return true;
}

// Bad file descriptors: negative, never opened, just closed, huge.
static bool write_ebadf(void)
{
	int fd = open("/dev/null", O_WRONLY);
	bool ok = fd >= 0;

	if (ok)
	{
		close(fd);
		ok = fails_with(-1, "x", 1, EBADF) && fails_with(-42, "x", 1, EBADF) &&
			 fails_with(fd, "x", 1, EBADF) && fails_with(4242, "x", 1, EBADF) &&
			 fails_with(0x7fffffff, "x", 1, EBADF) &&
			 fails_with(-1, "x", 0, EBADF) && same_as_libc(-1, "x", 1) &&
			 same_as_libc(fd, "x", 1);
	}
	return ok;
}

// fds that exist but are not open for writing.
static bool write_not_writable(void)
{
	int ro = open("/dev/null", O_RDONLY);
	int dir = open(".", O_RDONLY | O_DIRECTORY);
	int p[2];
	bool ok = ro >= 0 && dir >= 0 && pipe(p) == 0;

	if (ok)
	{
		ok = fails_with(ro, "x", 1, EBADF) && fails_with(dir, "x", 1, EBADF) &&
			 fails_with(p[0], "x", 1, EBADF) && same_as_libc(ro, "x", 1) &&
			 same_as_libc(dir, "x", 1) && same_as_libc(p[0], "x", 1);
		close(p[0]);
		close(p[1]);
	}
	if (ro >= 0)
		close(ro);
	if (dir >= 0)
		close(dir);
	return ok;
}

// Buffer the kernel cannot read: NULL, and a PROT_NONE guard page.
static bool write_efault(void)
{
	size_t page;
	char *mid = guarded_page(&page);
	int fd = open("/dev/null", O_WRONLY);
	int p[2];
	bool ok = mid && fd >= 0 && pipe(p) == 0;

	if (ok)
	{
		ok = fails_with(p[1], NULL, 1, EFAULT) &&
			 fails_with(p[1], mid - page, 16, EFAULT) &&
			 fails_with(p[1], mid + page, 16, EFAULT) &&
			 same_as_libc(p[1], NULL, 1) &&
			 same_as_libc(p[1], mid + page, 16)
			 // /dev/null never reads the buffer: success, like libc.
			 && same_as_libc(fd, NULL, 1) && pipe_empty(p[0]);
		close(p[0]);
		close(p[1]);
	}
	if (fd >= 0)
		close(fd);
	if (mid)
		release_page(mid, page);
	return ok;
}

// Writing to a pipe whose read end is closed: -1 / EPIPE (SIGPIPE ignored).
static bool write_epipe(void)
{
	int p[2];
	void (*old)(int) = signal(SIGPIPE, SIG_IGN);
	bool ok = pipe(p) == 0;

	if (ok)
	{
		close(p[0]);
		ok = fails_with(p[1], "x", 1, EPIPE) && same_as_libc(p[1], "x", 1);
		close(p[1]);
	}
	signal(SIGPIPE, old);
	return ok;
}

// Non-blocking pipe: fill it to capacity (partial write returns the real
// count, not count), then the next write fails with EAGAIN.
static bool write_partial_eagain(void)
{
	int p[2];
	char *buf = malloc(BIG_LEN);
	bool ok = buf && pipe(p) == 0;

	if (ok)
	{
		fcntl(p[1], F_SETFL, O_NONBLOCK);
		const int cap = fcntl(p[1], F_GETPIPE_SZ);
		fill_pattern(buf, BIG_LEN);
		// cap < BIG_LEN, so this is a short write of exactly cap bytes.
		const ssize_t r = ft_write(p[1], buf, BIG_LEN);
		ok = cap > 0 && (size_t)cap < BIG_LEN && r == cap &&
			 fails_with(p[1], buf, 1, EAGAIN) && same_as_libc(p[1], buf, 1);
		if (r != cap)
			fprintf(stderr, "  pipe cap=%d, short write returned %zd\n", cap,
					r);
		close(p[0]);
		close(p[1]);
	}
	free(buf);
	return ok;
}

// count == 0: returns 0, writes nothing, even with a NULL buffer.
static bool write_zero(void)
{
	int p[2];
	bool ok = pipe(p) == 0;

	if (ok)
	{
		ok = ft_write(p[1], "abc", 0) == 0 && ft_write(p[1], NULL, 0) == 0 &&
			 pipe_empty(p[0]);
		close(p[0]);
		close(p[1]);
	}
	return ok;
}

// errno must not be touched on success.
static bool write_success_keeps_errno(void)
{
	int fd = open("/dev/null", O_WRONLY);
	bool ok = fd >= 0;

	if (ok)
	{
		errno = ERRNO_SENTINEL;
		ok = ft_write(fd, "abc", 3) == 3 && errno == ERRNO_SENTINEL;
		close(fd);
	}
	return ok;
}

// A failing call followed by a succeeding one: the second must return its
// own count, not a stale -1.
static bool write_error_then_success(void)
{
	int fd = open("/dev/null", O_WRONLY);
	bool ok = fd >= 0;

	if (ok)
	{
		ok = fails_with(-1, "abc", 3, EBADF) && ft_write(fd, "abc", 3) == 3;
		close(fd);
	}
	return ok;
}

// Every length in [0, 64] at every buffer misalignment in [0, 15], through
// a pipe, with high bytes and nuls in the data.
static bool write_sweep(void)
{
	static char buf[128] __attribute__((aligned(64)));

	for (size_t len = 0; len <= 64; len++)
		for (size_t off = 0; off < 16; off++)
		{
			fill_pattern(buf, sizeof(buf));
			if (!write_pipe_ok(buf + off, len))
			{
				fprintf(stderr, "  first mismatch: len=%zu off=%zu\n", len,
						off);
				return false;
			}
		}
	return true;
}

// Counts around the 8/16-bit boundaries and well past 65535 to /dev/null
// (never short): catches a count passed or returned in a sub-register.
// Then BIG_LEN bytes into a regular file, read back byte for byte.
static bool write_big(void)
{
	static const size_t lens[] = {255, 256, 257, 65535, 65536, 65537, BIG_LEN};
	char *buf = malloc(BIG_LEN);
	char *back = malloc(BIG_LEN);
	int null = open("/dev/null", O_WRONLY);
	FILE *tmp = tmpfile();
	bool ok = buf && back && null >= 0 && tmp;

	if (ok)
	{
		fill_pattern(buf, BIG_LEN);
		for (size_t i = 0; i < sizeof(lens) / sizeof(*lens) && ok; i++)
			ok = ft_write(null, buf, lens[i]) == (ssize_t)lens[i];
		const int fd = fileno(tmp);
		ok = ok && ft_write(fd, buf, BIG_LEN) == (ssize_t)BIG_LEN &&
			 lseek(fd, 0, SEEK_SET) == 0 && read_all(fd, back, BIG_LEN) &&
			 memcmp(buf, back, BIG_LEN) == 0;
	}
	if (null >= 0)
		close(null);
	if (tmp)
		fclose(tmp);
	free(buf);
	free(back);
	return ok;
}

// Buffer ending on the last byte of a page and starting on the first byte of
// a page: any read outside [buf, buf + count) would EFAULT.
static bool write_page_guard(void)
{
	size_t page;
	char *mid = guarded_page(&page);
	bool ok = mid != NULL;

	for (size_t len = 0; len <= 64 && ok; len++)
	{
		fill_pattern(mid, page);
		ok = write_pipe_ok(mid + page - len, len) && write_pipe_ok(mid, len);
	}
	if (mid)
		release_page(mid, page);
	return ok;
}

// Successive writes on the same fd append in order (no internal state).
static bool write_sequence(void)
{
	int p[2];
	char got[12];
	bool ok = pipe(p) == 0;

	if (ok)
	{
		ok = ft_write(p[1], "Hell", 4) == 4 && ft_write(p[1], "o ", 2) == 2 &&
			 ft_write(p[1], "World!", 6) == 6 && read_all(p[0], got, 12) &&
			 memcmp(got, "Hello World!", 12) == 0;
		close(p[0]);
		close(p[1]);
	}
	return ok;
}

int main(void)
{
	printf("%ld\n", ft_write(1, ".", 1));
	ASSERT("ft_write classic", write_pipe_ok("Hello World!", 12));
	ASSERT("ft_write one byte", write_pipe_ok("a", 1));
	ASSERT("ft_write embedded nul and high bytes",
		   write_pipe_ok("ab\0\xff\x80\0cd", 8));
	ASSERT("ft_write count 0", write_zero());
	ASSERT("ft_write sequence", write_sequence());
	ASSERT("ft_write EBADF", write_ebadf());
	ASSERT("ft_write fd not open for writing", write_not_writable());
	ASSERT("ft_write EFAULT", write_efault());
	ASSERT("ft_write EPIPE", write_epipe());
	ASSERT("ft_write short write then EAGAIN", write_partial_eagain());
	ASSERT("ft_write keeps errno on success", write_success_keeps_errno());
	ASSERT("ft_write error then success", write_error_then_success());
	ASSERT("ft_write length/alignment sweep", write_sweep());
	ASSERT("ft_write long, count width", write_big());
	ASSERT("ft_write no read outside the buffer", write_page_guard());
}
