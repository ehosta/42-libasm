#define _DEFAULT_SOURCE
#include "../include/unit.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern size_t ft_strlen(const char *s);
extern char *ft_strcpy(char *dest, const char *src);

#define CPY_BUFSIZE 10016

// Non-zero, position-dependent pattern: a missing write, a missing terminator
// or a write past the terminator always shows up as a byte mismatch.
static void fill_pattern(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
		buf[i] = (char)(i % 251 + 1);
}

// Copies src into two identically pre-filled buffers (at dest_offset) with
// ft_strcpy and strcpy, then checks both buffers are byte-identical.
static bool strcpy_same(const char *src, size_t dest_offset)
{
	static char ft_buf[CPY_BUFSIZE];
	static char libc_buf[CPY_BUFSIZE];

	fill_pattern(ft_buf, CPY_BUFSIZE);
	fill_pattern(libc_buf, CPY_BUFSIZE);
	ft_strcpy(ft_buf + dest_offset, src);
	strcpy(libc_buf + dest_offset, src);
	return memcmp(ft_buf, libc_buf, CPY_BUFSIZE) == 0;
}

// Every length in [0, 64] for every src/dest misalignment in [0, 15].
static bool strcpy_sweep(void)
{
	char src[128];

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
					fprintf(stderr, "  first mismatch: len=%zu src_off=%zu "
									"dst_off=%zu\n", len, src_off, dst_off);
					return false;
				}
			}
	return true;
}

// Puts src's terminator on the last byte before a PROT_NONE page: any read
// past the terminator segfaults.
static bool strcpy_page_guard(void)
{
	const size_t page = (size_t)sysconf(_SC_PAGESIZE);
	char *map = mmap(NULL, page * 2, PROT_READ | PROT_WRITE,
					 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (map == MAP_FAILED || mprotect(map + page, page, PROT_NONE))
		return false;
	bool ok = true;
	for (size_t len = 0; len <= 64 && ok; len++)
	{
		char *src = map + page - len - 1;
		memset(src, 'g', len);
		src[len] = 0;
		ok = strcpy_same(src, 0);
	}
	munmap(map, page * 2);
	return ok;
}

int main(void)
{
	char big_buffer[10001];
	memset(big_buffer, 'a', 10000);
	big_buffer[10000] = 0;

	// STRLEN
	ASSERT_SAME("classic", ft_strlen, strlen, "Hello World!");
	ASSERT_SAME("empty", ft_strlen, strlen, "");
	ASSERT_SAME("one char", ft_strlen, strlen, "a");
	ASSERT_SAME("high bytes", ft_strlen, strlen, "\xff\x80\xfe");
	ASSERT_SAME("utf-8", ft_strlen, strlen, "héllo");
	ASSERT_SAME("embedded nul", ft_strlen, strlen, "abc\0def");
	ASSERT_SAME("long", ft_strlen, strlen, big_buffer);
	ASSERT_SAME("unaligned", ft_strlen, strlen, big_buffer + 3);

	// STRCPY
	char dest[32];
	ASSERT("ft_strcpy returns dest", ft_strcpy(dest, "abc") == dest);
	ASSERT("ft_strcpy returns dest (empty)", ft_strcpy(dest, "") == dest);
	ASSERT("ft_strcpy classic", strcpy_same("Hello World!", 0));
	ASSERT("ft_strcpy empty", strcpy_same("", 0));
	ASSERT("ft_strcpy high bytes", strcpy_same("\xff\x80\x01", 0));
	ASSERT("ft_strcpy stops at first nul", strcpy_same("abc\0def", 0));
	ASSERT("ft_strcpy long", strcpy_same(big_buffer, 0));
	ASSERT("ft_strcpy length/alignment sweep", strcpy_sweep());
	ASSERT("ft_strcpy no read past nul", strcpy_page_guard());
}
