#include <stdio.h>

#define COLOR_RED "\e[38;2;255;0;0m"
#define COLOR_RESET "\e[0m"

extern void print_hello(void);

int	main(void)
{
	printf(COLOR_RED "print_hello" COLOR_RESET "\n");
	fflush(stdin);
	print_hello();
}
