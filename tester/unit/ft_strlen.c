#define _DEFAULT_SOURCE
#include "../include/unit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

extern size_t ft_strlen(const char *s);

int main(void)
{
	char big_buffer[10001];
	memset(big_buffer, 'a', 10000);
	big_buffer[10000] = 0;

	ASSERT_SAME("classic", ft_strlen, strlen, "Hello World!");
	ASSERT_SAME("empty", ft_strlen, strlen, "");
	ASSERT_SAME("one char", ft_strlen, strlen, "a");
	ASSERT_SAME("high bytes", ft_strlen, strlen, "\xff\x80\xfe");
	ASSERT_SAME("utf-8", ft_strlen, strlen, "héllo");
	ASSERT_SAME("embedded nul", ft_strlen, strlen, "abc\0def");
	ASSERT_SAME("long", ft_strlen, strlen, big_buffer);
	ASSERT_SAME("unaligned", ft_strlen, strlen, big_buffer + 3);
}
