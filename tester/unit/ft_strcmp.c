#define _DEFAULT_SOURCE
#include "../include/unit.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern int ft_strcmp(const char *s1, const char *s2);

#define BIG_LEN (((size_t)1 << 20) + 3)

// man 3 strcmp only specifies the sign of the result, never its value.
static int sign(int x) { return (x > 0) - (x < 0); }

// ft_strcmp and strcmp agree on the sign, both ways round.
static bool same_sign(const char *s1, const char *s2)
{
	return sign(ft_strcmp(s1, s2)) == sign(strcmp(s1, s2)) &&
		   sign(ft_strcmp(s2, s1)) == sign(strcmp(s2, s1));
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

static void fill_alpha(char *s, size_t len)
{
	for (size_t i = 0; i < len; i++)
		s[i] = (char)('a' + i % 26);
	s[len] = 0;
}

// Every pair of bytes, compared as unsigned char (man 3 strcmp), both as the
// first byte and after a common prefix. Expected sign comes from the spec,
// not from libc. Catches signed-char comparisons and an 8-bit subtraction
// whose result is read back as a signed byte.
static bool strcmp_all_bytes(void)
{
	for (int x = 0; x < 256; x++)
		for (int y = 0; y < 256; y++)
		{
			const char a[] = {(char)x, 0};
			const char b[] = {(char)y, 0};
			const char pa[] = {'k', (char)x, 0};
			const char pb[] = {'k', (char)y, 0};
			const int expected = (x > y) - (x < y);

			if (sign(ft_strcmp(a, b)) != expected ||
				sign(ft_strcmp(pa, pb)) != expected)
			{
				fprintf(stderr, "  first mismatch: 0x%02x vs 0x%02x\n", x, y);
				return false;
			}
		}
	return true;
}

static bool sweep_fail(size_t len, size_t o1, size_t o2, size_t p, int v)
{
	fprintf(stderr,
			"  first mismatch: len=%zu off1=%zu off2=%zu diff_at=%zu %s\n", len,
			o1, o2, p,
			v < 0 ? "(equal)"
			: v	  ? "(|0x80)"
				  : "(+1)");
	return false;
}

// Every length in [0, 64] for every s1/s2 misalignment in [0, 15]:
// - equal strings followed by different bytes after the nul -> 0
// - a difference at every position (+1 or high bit set) -> s1 < s2
// - s2 longer by one byte (s1 is a strict prefix)        -> s1 < s2
// each checked both ways round.
static bool strcmp_sweep(void)
{
	static char b1[128] __attribute__((aligned(64)));
	static char b2[128] __attribute__((aligned(64)));

	for (size_t len = 0; len <= 64; len++)
		for (size_t o1 = 0; o1 < 16; o1++)
			for (size_t o2 = 0; o2 < 16; o2++)
			{
				char *s1 = b1 + o1;
				char *s2 = b2 + o2;

				memset(b1, 'y', sizeof(b1));
				memset(b2, 'z', sizeof(b2));
				fill_alpha(s1, len);
				fill_alpha(s2, len);
				if (ft_strcmp(s1, s2) != 0)
					return sweep_fail(len, o1, o2, 0, -1);
				for (size_t p = 0; p <= len; p++)
					for (int v = 0; v < 2; v++)
					{
						const char saved = s2[p];
						const char saved_next = s2[p + 1];

						if (p == len)
						{
							s2[p] = v ? '\x80' : 'a';
							s2[p + 1] = 0;
						}
						else
							s2[p] =
								v ? (char)(s1[p] | 0x80) : (char)(s1[p] + 1);
						const bool ok = sign(ft_strcmp(s1, s2)) == -1 &&
										sign(ft_strcmp(s2, s1)) == 1;
						s2[p] = saved;
						s2[p + 1] = saved_next;
						if (!ok)
							return sweep_fail(len, o1, o2, p, v);
					}
			}
	return true;
}

// 1 MB strings: equal, a difference around the 8/16-bit boundaries and at
// the very end, and a strict prefix. Also checks neither string is modified.
static bool strcmp_big(void)
{
	static const size_t diffs[] = {255, 256, 65535, 65536, BIG_LEN - 1};
	char *a = malloc(BIG_LEN + 1);
	char *b = malloc(BIG_LEN + 1);
	bool ok = a && b;

	if (ok)
	{
		fill_alpha(a, BIG_LEN);
		fill_alpha(b, BIG_LEN);
		ok = ft_strcmp(a, b) == 0;
		for (size_t i = 0; i < sizeof(diffs) / sizeof(*diffs) && ok; i++)
		{
			b[diffs[i]]++;
			ok = sign(ft_strcmp(a, b)) == -1 && sign(ft_strcmp(b, a)) == 1;
			b[diffs[i]]--;
		}
		b[BIG_LEN - 1] = 0;
		ok = ok && sign(ft_strcmp(a, b)) == 1 && sign(ft_strcmp(b, a)) == -1;
		b[BIG_LEN - 1] = a[BIG_LEN - 1];
		for (size_t i = 0; i < BIG_LEN && ok; i++)
			ok = a[i] == (char)('a' + i % 26) && a[i] == b[i];
	}
	free(a);
	free(b);
	return ok;
}

// Strings ending on the last byte of a page or starting on the first byte of
// a page, on two separate guarded pages: any read past a nul or before the
// start of a string faults. Includes the case where s1 is a prefix of s2,
// which must stop at s1's nul.
static bool strcmp_page_guard(void)
{
	size_t page;
	char *p1 = guarded_page(&page);
	char *p2 = guarded_page(&page);
	bool ok = p1 && p2;

	for (size_t len = 0; len <= 64 && ok; len++)
	{
		char *end1 = p1 + page - len - 1;
		char *end2 = p2 + page - len - 1;

		fill_alpha(end1, len);
		fill_alpha(end2, len);
		fill_alpha(p1, len);
		fill_alpha(p2, len + 1);
		ok = ft_strcmp(end1, end2) == 0			// both end on a page boundary
			 && ft_strcmp(end1, p1) == 0		// start-of-page vs end-of-page
			 && sign(ft_strcmp(end1, p2)) == -1 // s1 prefix, ends at boundary
			 && sign(ft_strcmp(p2, end1)) == 1;
	}
	if (p1)
		release_page(p1, page);
	if (p2)
		release_page(p2, page);
	return ok;
}

int main(void)
{
	const char *same = "same pointer";

	ASSERT("ft_strcmp equal", ft_strcmp("Hello", "Hello") == 0);
	ASSERT("ft_strcmp same pointer", ft_strcmp(same, same) == 0);
	ASSERT("ft_strcmp both empty", ft_strcmp("", "") == 0);
	ASSERT("ft_strcmp classic", same_sign("Hello", "Hellp"));
	ASSERT("ft_strcmp first char", same_sign("abc", "bbc"));
	ASSERT("ft_strcmp empty vs non-empty", same_sign("", "a"));
	ASSERT("ft_strcmp prefix", same_sign("abc", "abcd"));
	ASSERT("ft_strcmp high byte is greater",
		   sign(ft_strcmp("\x80", "\x7f")) == 1);
	ASSERT("ft_strcmp 0xff vs 0x01", sign(ft_strcmp("\xff", "\x01")) == 1);
	ASSERT("ft_strcmp high byte after prefix",
		   sign(ft_strcmp("a\x01", "a\xff")) == -1);
	ASSERT("ft_strcmp 0xff vs empty", sign(ft_strcmp("\xff", "")) == 1);
	ASSERT("ft_strcmp utf-8", same_sign("héllo", "hello"));
	ASSERT("ft_strcmp ignores bytes after nul",
		   ft_strcmp("abc\0x", "abc\0y") == 0);
	ASSERT("ft_strcmp all byte pairs (unsigned)", strcmp_all_bytes());
	ASSERT("ft_strcmp length/alignment/position sweep", strcmp_sweep());
	ASSERT("ft_strcmp long, counter width", strcmp_big());
	ASSERT("ft_strcmp no read outside the strings", strcmp_page_guard());
}
