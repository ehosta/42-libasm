#include <stdlib.h>
#include <stdio.h>

extern size_t	ft_strlen(const char *s);

int	main(void)
{
	printf("Len: %ld\n", ft_strlen("Hello world!"));
}
