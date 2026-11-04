#define _GNU_SOURCE
#include "../include/unit.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <unistd.h>

extern ssize_t ft_read(int fd, void *buf, size_t count);

#define BIG_LEN (((size_t)1 << 20) + 3)

// Sentinel errno value: lets us see whether ft_read touched errno at all.
#define ERRNO_SENTINEL 4242

// Byte used to fill destination buffers before a read: any byte ft_read
// should not have written still holds it afterwards.
#define CANARY 0x5a

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
// (read must not care about string semantics).
static void fill_pattern(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
		buf[i] = (char)(i * 7 + 3);
}

// Every byte of buf[0..n) still holds the canary.
static bool untouched(const char *buf, size_t n)
{
	for (size_t i = 0; i < n; i++)
		if ((unsigned char)buf[i] != CANARY)
			return false;
	return true;
}

// Writes exactly n bytes of buf to fd (helper side: uses libc write).
static bool write_all(int fd, const char *buf, size_t n)
{
	size_t done = 0;

	while (done < n)
	{
		ssize_t w = write(fd, buf + done, n - done);
		if (w <= 0)
			return false;
		done += (size_t)w;
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

// Puts src[0..n) in a pipe, reads it back with ft_read into a canary-filled
// buffer of n + 16 bytes and checks: return value is exactly n (full 64-bit
// ssize_t), the n bytes match, the 16 bytes after them are untouched, and
// the pipe is drained.
static bool read_pipe_ok(const char *src, size_t n)
{
	int p[2];
	char *dst = malloc(n + 16);
	bool ok = dst && pipe(p) == 0;

	if (ok)
	{
		memset(dst, CANARY, n + 16);
		ok = write_all(p[1], src, n) && ft_read(p[0], dst, n) == (ssize_t)n &&
			 memcmp(dst, src, n) == 0 && untouched(dst + n, 16) &&
			 pipe_empty(p[0]);
		close(p[0]);
		close(p[1]);
	}
	free(dst);
	return ok;
}

// ft_read and read agree on both the return value and errno.
// errno is reset to a sentinel before each call, so a missing or wrong
// errno write shows up. Only meant for calls that fail or consume nothing.
static bool same_as_libc(int fd, void *buf, size_t n)
{
	errno = ERRNO_SENTINEL;
	const ssize_t ft = ft_read(fd, buf, n);
	const int ft_err = errno;
	errno = ERRNO_SENTINEL;
	const ssize_t lc = read(fd, buf, n);
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
static bool fails_with(int fd, void *buf, size_t n, int err)
{
	errno = ERRNO_SENTINEL;
	const ssize_t r = ft_read(fd, buf, n);
	const int e = errno;

	if (r != -1 || e != err)
	{
		fprintf(stderr, "  fd=%d n=%zu: got ret=%zd errno=%d, want -1/%d\n", fd,
				n, r, e, err);
		return false;
	}
	return true;
}

// count == 0: returns 0, touches nothing and consumes nothing, even with a
// NULL buffer and data waiting.
static bool read_zero(void)
{
	int p[2];
	char buf[8];
	bool ok = pipe(p) == 0;

	if (ok)
	{
		memset(buf, CANARY, sizeof(buf));
		ok = write_all(p[1], "abc", 3) && ft_read(p[0], buf, 0) == 0 &&
			 ft_read(p[0], NULL, 0) == 0 && untouched(buf, sizeof(buf)) &&
			 read(p[0], buf, 3) == 3 && memcmp(buf, "abc", 3) == 0;
		close(p[0]);
		close(p[1]);
	}
	return ok;
}

// End of file returns 0: empty pipe whose write end is closed, empty file,
// /dev/null, and a regular file once fully read.
static bool read_eof(void)
{
	int p[2];
	char buf[8];
	int null = open("/dev/null", O_RDONLY);
	FILE *tmp = tmpfile();
	bool ok = null >= 0 && tmp && pipe(p) == 0;

	if (ok)
	{
		const int fd = fileno(tmp);
		memset(buf, CANARY, sizeof(buf));
		close(p[1]);
		ok = ft_read(p[0], buf, sizeof(buf)) == 0 &&
			 ft_read(null, buf, sizeof(buf)) == 0 &&
			 ft_read(fd, buf, sizeof(buf)) == 0 &&
			 untouched(buf, sizeof(buf)) && write_all(fd, "xyz", 3) &&
			 lseek(fd, 0, SEEK_SET) == 0 &&
			 ft_read(fd, buf, sizeof(buf)) == 3 &&
			 ft_read(fd, buf, sizeof(buf)) == 0;
		close(p[0]);
	}
	if (null >= 0)
		close(null);
	if (tmp)
		fclose(tmp);
	return ok;
}

// Fewer bytes available than requested: returns what is there, writes only
// that many bytes into the buffer.
static bool read_short(void)
{
	int p[2];
	char buf[64];
	bool ok = pipe(p) == 0;

	if (ok)
	{
		memset(buf, CANARY, sizeof(buf));
		ok = write_all(p[1], "hello", 5) &&
			 ft_read(p[0], buf, sizeof(buf)) == 5 &&
			 memcmp(buf, "hello", 5) == 0 &&
			 untouched(buf + 5, sizeof(buf) - 5);
		close(p[0]);
		close(p[1]);
	}
	return ok;
}

// Successive reads consume the data in order, on a pipe and on a regular
// file (the file offset advances by exactly what was returned).
static bool read_sequence(void)
{
	int p[2];
	char got[12];
	FILE *tmp = tmpfile();
	bool ok = tmp && pipe(p) == 0;

	if (ok)
	{
		ok = write_all(p[1], "Hello World!", 12) &&
			 ft_read(p[0], got, 4) == 4 && ft_read(p[0], got + 4, 2) == 2 &&
			 ft_read(p[0], got + 6, 6) == 6 &&
			 memcmp(got, "Hello World!", 12) == 0 && pipe_empty(p[0]);
		close(p[0]);
		close(p[1]);
		const int fd = fileno(tmp);
		memset(got, 0, sizeof(got));
		ok = ok && write_all(fd, "Hello World!", 12) &&
			 lseek(fd, 0, SEEK_SET) == 0 && ft_read(fd, got, 5) == 5 &&
			 lseek(fd, 0, SEEK_CUR) == 5 && ft_read(fd, got + 5, 7) == 7 &&
			 lseek(fd, 0, SEEK_CUR) == 12 &&
			 memcmp(got, "Hello World!", 12) == 0;
	}
	if (tmp)
		fclose(tmp);
	return ok;
}

// Bad file descriptors: negative, never opened, just closed, huge.
static bool read_ebadf(void)
{
	char buf[4];
	int fd = open("/dev/null", O_RDONLY);
	bool ok = fd >= 0;

	if (ok)
	{
		close(fd);
		ok = fails_with(-1, buf, 1, EBADF) && fails_with(-42, buf, 1, EBADF) &&
			 fails_with(fd, buf, 1, EBADF) && fails_with(4242, buf, 1, EBADF) &&
			 fails_with(0x7fffffff, buf, 1, EBADF) &&
			 fails_with(-1, buf, 0, EBADF) && same_as_libc(-1, buf, 1) &&
			 same_as_libc(fd, buf, 1);
	}
	return ok;
}

// fds that exist but are not open for reading.
static bool read_not_readable(void)
{
	char buf[4];
	int wo = open("/dev/null", O_WRONLY);
	int p[2];
	bool ok = wo >= 0 && pipe(p) == 0;

	if (ok)
	{
		ok = fails_with(wo, buf, 1, EBADF) && fails_with(p[1], buf, 1, EBADF) &&
			 same_as_libc(wo, buf, 1) && same_as_libc(p[1], buf, 1);
		close(p[0]);
		close(p[1]);
	}
	if (wo >= 0)
		close(wo);
	return ok;
}

// Reading a directory: -1 / EISDIR.
static bool read_eisdir(void)
{
	char buf[16];
	int dir = open(".", O_RDONLY | O_DIRECTORY);
	bool ok = dir >= 0;

	if (ok)
	{
		ok = fails_with(dir, buf, sizeof(buf), EISDIR) &&
			 same_as_libc(dir, buf, sizeof(buf));
		close(dir);
	}
	return ok;
}

// Buffer the kernel cannot write into: NULL, and both PROT_NONE guard pages.
static bool read_efault(void)
{
	size_t page;
	char *mid = guarded_page(&page);
	int p[2];
	bool ok = mid && pipe(p) == 0;

	if (ok)
	{
		ok = write_all(p[1], "0123456789abcdef", 16) &&
			 fails_with(p[0], NULL, 1, EFAULT) &&
			 fails_with(p[0], mid - page, 16, EFAULT) &&
			 fails_with(p[0], mid + page, 16, EFAULT) &&
			 same_as_libc(p[0], NULL, 1) && same_as_libc(p[0], mid + page, 16);
		close(p[0]);
		close(p[1]);
	}
	if (mid)
		release_page(mid, page);
	return ok;
}

// Non-blocking pipe with nothing in it (write end still open): -1 / EAGAIN.
// Then data arrives and the next call succeeds.
static bool read_eagain(void)
{
	int p[2];
	char buf[8];
	bool ok = pipe(p) == 0;

	if (ok)
	{
		fcntl(p[0], F_SETFL, O_NONBLOCK);
		ok = fails_with(p[0], buf, sizeof(buf), EAGAIN) &&
			 same_as_libc(p[0], buf, sizeof(buf)) && write_all(p[1], "ok", 2) &&
			 ft_read(p[0], buf, sizeof(buf)) == 2 && memcmp(buf, "ok", 2) == 0;
		close(p[0]);
		close(p[1]);
	}
	return ok;
}

// An eventfd only accepts reads of at least 8 bytes: -1 / EINVAL below that.
static bool read_einval(void)
{
	uint64_t v = 0;
	int efd = eventfd(3, 0);
	bool ok = efd >= 0;

	if (ok)
	{
		ok = fails_with(efd, &v, 4, EINVAL) && fails_with(efd, &v, 7, EINVAL) &&
			 same_as_libc(efd, &v, 1) && ft_read(efd, &v, 8) == 8 && v == 3;
		close(efd);
	}
	return ok;
}

// errno must not be touched on success, nor on end of file.
static bool read_success_keeps_errno(void)
{
	char buf[8];
	int fd = open("/dev/zero", O_RDONLY);
	int null = open("/dev/null", O_RDONLY);
	bool ok = fd >= 0 && null >= 0;

	if (ok)
	{
		errno = ERRNO_SENTINEL;
		ok = ft_read(fd, buf, sizeof(buf)) == (ssize_t)sizeof(buf) &&
			 errno == ERRNO_SENTINEL && ft_read(null, buf, sizeof(buf)) == 0 &&
			 errno == ERRNO_SENTINEL;
	}
	if (fd >= 0)
		close(fd);
	if (null >= 0)
		close(null);
	return ok;
}

// A failing call followed by a succeeding one: the second must return its
// own count, not a stale -1.
static bool read_error_then_success(void)
{
	char buf[8];
	int fd = open("/dev/zero", O_RDONLY);
	bool ok = fd >= 0;

	if (ok)
	{
		ok = fails_with(-1, buf, 3, EBADF) && ft_read(fd, buf, 3) == 3;
		close(fd);
	}
	return ok;
}

// Every length in [0, 64] at every destination misalignment in [0, 15],
// through a pipe, with high bytes and nuls in the data.
static bool read_sweep(void)
{
	char src[64];

	fill_pattern(src, sizeof(src));
	for (size_t len = 0; len <= 64; len++)
		for (size_t off = 0; off < 16; off++)
		{
			int p[2];
			static char dst[128] __attribute__((aligned(64)));
			bool ok = pipe(p) == 0;

			if (ok)
			{
				memset(dst, CANARY, sizeof(dst));
				ok = write_all(p[1], src, len) &&
					 ft_read(p[0], dst + off, len) == (ssize_t)len &&
					 untouched(dst, off) && memcmp(dst + off, src, len) == 0 &&
					 untouched(dst + off + len, sizeof(dst) - off - len);
				close(p[0]);
				close(p[1]);
			}
			if (!ok)
			{
				fprintf(stderr, "  first mismatch: len=%zu off=%zu\n", len,
						off);
				return false;
			}
		}
	return true;
}

// Counts around the 8/16-bit boundaries and well past 65535 from /dev/zero
// (never short): catches a count passed or returned in a sub-register.
// Then BIG_LEN bytes from a regular file, compared byte for byte.
static bool read_big(void)
{
	static const size_t lens[] = {255, 256, 257, 65535, 65536, 65537, BIG_LEN};
	char *src = malloc(BIG_LEN);
	char *dst = malloc(BIG_LEN);
	int zero = open("/dev/zero", O_RDONLY);
	FILE *tmp = tmpfile();
	bool ok = src && dst && zero >= 0 && tmp;

	if (ok)
	{
		for (size_t i = 0; i < sizeof(lens) / sizeof(*lens) && ok; i++)
		{
			memset(dst, CANARY, lens[i]);
			ok = ft_read(zero, dst, lens[i]) == (ssize_t)lens[i] &&
				 dst[0] == 0 && dst[lens[i] - 1] == 0;
		}
		const int fd = fileno(tmp);
		fill_pattern(src, BIG_LEN);
		memset(dst, CANARY, BIG_LEN);
		ok = ok && write_all(fd, src, BIG_LEN) && lseek(fd, 0, SEEK_SET) == 0 &&
			 ft_read(fd, dst, BIG_LEN) == (ssize_t)BIG_LEN &&
			 memcmp(src, dst, BIG_LEN) == 0;
	}
	if (zero >= 0)
		close(zero);
	if (tmp)
		fclose(tmp);
	free(src);
	free(dst);
	return ok;
}

// count with bits set above bit 31: only 10 bytes are in the pipe, so a
// correct read returns 10. A count cut down to its low 32 bits (3) would
// return 3. The buffer is on the heap on purpose: the kernel refuses
// (EFAULT) a range [buf, buf + count) that runs past the top of user
// memory, and a stack buffer + 4 GiB can do that.
static bool read_count_high_bits(void)
{
	int p[2];
	char *buf = malloc(32);
	bool ok = buf && pipe(p) == 0;

	if (ok)
	{
		memset(buf, CANARY, 32);
		const bool filled = write_all(p[1], "0123456789", 10);
		errno = ERRNO_SENTINEL;
		const ssize_t r = ft_read(p[0], buf, ((size_t)1 << 32) + 3);
		ok = filled && r == 10 && memcmp(buf, "0123456789", 10) == 0 &&
			 untouched(buf + 10, 22);
		if (!ok)
			fprintf(stderr, "  count 2^32+3: got ret=%zd errno=%d, want 10\n",
					r, errno);
		close(p[0]);
		close(p[1]);
	}
	free(buf);
	return ok;
}

// Destination ending on the last byte of a page and starting on the first
// byte of a page: any write outside [buf, buf + count) would EFAULT.
static bool read_page_guard(void)
{
	size_t page;
	char src[64];
	char *mid = guarded_page(&page);
	bool ok = mid != NULL;

	fill_pattern(src, sizeof(src));
	for (size_t len = 0; len <= 64 && ok; len++)
	{
		int p[2];
		ok = pipe(p) == 0;
		if (!ok)
			break;
		ok = write_all(p[1], src, len) &&
			 ft_read(p[0], mid + page - len, len) == (ssize_t)len &&
			 memcmp(mid + page - len, src, len) == 0 &&
			 write_all(p[1], src, len) &&
			 ft_read(p[0], mid, len) == (ssize_t)len &&
			 memcmp(mid, src, len) == 0;
		close(p[0]);
		close(p[1]);
	}
	if (mid)
		release_page(mid, page);
	return ok;
}

int main(void)
{
	ASSERT("ft_read classic", read_pipe_ok("Hello World!", 12));
	ASSERT("ft_read one byte", read_pipe_ok("a", 1));
	ASSERT("ft_read embedded nul and high bytes",
		   read_pipe_ok("ab\0\xff\x80\0cd", 8));
	ASSERT("ft_read count 0", read_zero());
	ASSERT("ft_read end of file", read_eof());
	ASSERT("ft_read short read", read_short());
	ASSERT("ft_read sequence and file offset", read_sequence());
	ASSERT("ft_read EBADF", read_ebadf());
	ASSERT("ft_read fd not open for reading", read_not_readable());
	ASSERT("ft_read EISDIR", read_eisdir());
	ASSERT("ft_read EFAULT", read_efault());
	ASSERT("ft_read EAGAIN then data", read_eagain());
	ASSERT("ft_read EINVAL", read_einval());
	ASSERT("ft_read keeps errno on success", read_success_keeps_errno());
	ASSERT("ft_read error then success", read_error_then_success());
	ASSERT("ft_read length/alignment sweep", read_sweep());
	ASSERT("ft_read long, count width", read_big());
	ASSERT("ft_read count with high bits", read_count_high_bits());
	ASSERT("ft_read no write outside the buffer", read_page_guard());
}
