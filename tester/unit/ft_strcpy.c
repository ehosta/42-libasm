#define _DEFAULT_SOURCE
#include "../include/unit.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern char *ft_strcpy(char *dest, const char *src);

#define BIG_LEN (((size_t)1 << 20) + 3)
// Bytes checked after the terminator: covers stores up to 64 bytes wide.
#define GUARD 64
#define CPY_BUFSIZE (15 + BIG_LEN + 1 + GUARD)

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

// Non-zero, position-dependent pattern: a missing write, a missing terminator
// or a write past the terminator always shows up as a byte mismatch.
static void fill_pattern(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
		buf[i] = (char)(i % 251 + 1);
}

// Copies src at dest_offset into two identically pre-filled buffers with
// ft_strcpy and strcpy. Checks the return value is dest, that both buffers
// match from their start to GUARD bytes past the terminator, and that src is
// left untouched.
static bool strcpy_same(const char *src, size_t dest_offset)
{
	static char ft_buf[CPY_BUFSIZE];
	static char libc_buf[CPY_BUFSIZE];
	static char src_copy[BIG_LEN + 1];
	const size_t len = strlen(src);
	const size_t window = dest_offset + len + 1 + GUARD;

	memcpy(src_copy, src, len + 1);
	fill_pattern(ft_buf, window);
	fill_pattern(libc_buf, window);
	if (ft_strcpy(ft_buf + dest_offset, src) != ft_buf + dest_offset)
		return false;
	strcpy(libc_buf + dest_offset, src);
	return memcmp(ft_buf, libc_buf, window) == 0 &&
		   memcmp(src, src_copy, len + 1) == 0;
}

// Every length in [0, 64] for every src/dest misalignment in [0, 15].
static bool strcpy_sweep(void)
{
	static char src[128] __attribute__((aligned(64)));

	for (size_t len = 0; len <= 64; len++)
		for (size_t src_off = 0; src_off < 16; src_off++)
			for (size_t dst_off = 0; dst_off < 16; dst_off++)
			{
				memset(src, 'z', sizeof(src));
				for (size_t i = 0; i < len; i++)
					src[src_off + i] = (char)('a' + i % 26);
				src[src_off + len] = 0;
				if (!strcpy_same(src + src_off, dst_off))
				{
					fprintf(stderr,
							"  first mismatch: len=%zu src_off=%zu "
							"dst_off=%zu\n",
							len, src_off, dst_off);
					return false;
				}
			}
	return true;
}

// Length well past 65535 (catches an 8/16-bit counter), non-uniform content.
static bool strcpy_big(size_t src_off, size_t dst_off)
{
	char *src = malloc(BIG_LEN + 16 + 1);
	bool ok;

	if (!src)
		return false;
	for (size_t i = 0; i < BIG_LEN + 16; i++)
		src[i] = (char)('a' + i % 26);
	src[src_off + BIG_LEN] = 0;
	ok = strcpy_same(src + src_off, dst_off);
	free(src);
	return ok;
}

// src at the end / start of a guarded page: reads outside src fault.
// dest at the end / start of a guarded page (exactly len + 1 bytes available
// before the guard): writes outside dest fault.
static bool strcpy_page_guard(void)
{
	size_t page;
	char *src_pg = guarded_page(&page);
	char *dst_pg = guarded_page(&page);
	bool ok = src_pg && dst_pg;

	for (size_t len = 0; len <= 64 && ok; len++)
	{
		char *src_end = src_pg + page - len - 1;
		char *dst_end = dst_pg + page - len - 1;

		for (size_t i = 0; i < len; i++)
		{
			src_end[i] = (char)('a' + i % 26);
			src_pg[i] = (char)('A' + i % 26);
		}
		src_end[len] = 0;
		src_pg[len] = 0;
		ok = ft_strcpy(dst_end, src_end) == dst_end &&
			 memcmp(dst_end, src_end, len + 1) == 0 &&
			 ft_strcpy(dst_pg, src_pg) == dst_pg &&
			 memcmp(dst_pg, src_pg, len + 1) == 0;
	}
	if (src_pg)
		release_page(src_pg, page);
	if (dst_pg)
		release_page(dst_pg, page);
	return ok;
}

int main(void)
{
	char dest[32];

	ASSERT("ft_strcpy returns dest", ft_strcpy(dest, "abc") == dest);
	ASSERT("ft_strcpy returns dest (empty)", ft_strcpy(dest, "") == dest);
	ASSERT("ft_strcpy classic", strcpy_same("Hello World!", 0));
	ASSERT("ft_strcpy empty", strcpy_same("", 0));
	ASSERT("ft_strcpy high bytes", strcpy_same("\xff\x80\x01", 0));
	ASSERT("ft_strcpy utf-8", strcpy_same("héllo", 0));
	ASSERT("ft_strcpy stops at first nul", strcpy_same("abc\0def", 0));
	ASSERT("ft_strcpy length/alignment sweep", strcpy_sweep());
	ASSERT("ft_strcpy long, aligned", strcpy_big(0, 0));
	ASSERT("ft_strcpy long, unaligned", strcpy_big(3, 5));
	ASSERT("ft_strcpy no access outside src/dest", strcpy_page_guard());
}
