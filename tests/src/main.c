#include "insider.h"
#include <stdlib.h>
#include <stdio.h>

int	main(int argc, char **argv)
{
	char	**res = split("Hello, I am       a  \t\tMONSTER\t.", " \t");
	size_t	i = 0;
	while (res[i])
	{
		printf("%s", res[i]);
		i++;
	}
	(void)argc;
	(void)argv;
	return 0;
}
