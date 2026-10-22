#define _DEFAULT_SOURCE
#include "../include/unit.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern size_t ft_strlen(const char *s);

#define BIG_LEN ((size_t)1 << 20)

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

// Every length in [0, 64] at every misalignment in [0, 15], with non-nul
// bytes after the terminator.
static bool strlen_sweep(void)
{
	static char buf[128] __attribute__((aligned(64)));

	for (size_t len = 0; len <= 64; len++)
		for (size_t off = 0; off < 16; off++)
		{
			memset(buf, 'z', sizeof(buf));
			buf[off + len] = 0;
			if (ft_strlen(buf + off) != len)
			{
				fprintf(stderr, "  first mismatch: len=%zu off=%zu\n", len,
						off);
				return false;
			}
		}
	return true;
}

// Lengths around the 8/16-bit boundaries and well past 65535: catches a
// counter kept in al/ax or a result returned in a sub-register of rax.
// Also checks the string is left untouched.
static bool strlen_big(void)
{
	static const size_t lens[] = {255, 256, 257, 65535, 65536, 65537, BIG_LEN};
	char *buf = malloc(BIG_LEN + 1);
	bool ok = true;

	if (!buf)
		return false;
	memset(buf, 'a', BIG_LEN);
	for (size_t i = 0; i < sizeof(lens) / sizeof(*lens) && ok; i++)
	{
		buf[lens[i]] = 0;
		ok = ft_strlen(buf) == lens[i];
		buf[lens[i]] = 'a';
	}
	for (size_t i = 0; i < BIG_LEN && ok; i++)
		ok = buf[i] == 'a';
	free(buf);
	return ok;
}

// Strings ending on the last byte of a page (reads past the nul fault) and
// starting on the first byte of a page (reads before s fault).
static bool strlen_page_guard(void)
{
	size_t page;
	char *mid = guarded_page(&page);
	bool ok = true;

	if (!mid)
		return false;
	for (size_t len = 0; len <= 64 && ok; len++)
	{
		char *end = mid + page - len - 1;
		memset(end, 'g', len);
		end[len] = 0;
		memset(mid, 'g', len);
		mid[len] = 0;
		ok = ft_strlen(end) == len && ft_strlen(mid) == len;
	}
	release_page(mid, page);
	return ok;
}

int main(void)
{
	ASSERT_SAME("classic", ft_strlen, strlen, "Hello World!");
	ASSERT_SAME("empty", ft_strlen, strlen, "");
	ASSERT_SAME("one char", ft_strlen, strlen, "a");
	ASSERT_SAME("high bytes", ft_strlen, strlen, "\xff\x80\xfe");
	ASSERT_SAME("utf-8", ft_strlen, strlen, "héllo");
	ASSERT_SAME("embedded nul", ft_strlen, strlen, "abc\0def");
	ASSERT("length/alignment sweep", strlen_sweep());
	ASSERT("long, counter width", strlen_big());
	ASSERT("no read outside the string", strlen_page_guard());
}
