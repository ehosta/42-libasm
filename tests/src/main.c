#include "insider.h"
#include <stdlib.h>
#include <stdio.h>

int	main(int argc, char **argv)
{
	char	**res = split(getenv("PATH"), ":");
	size_t	i = 0;
	while (res[i])
	{
		printf("%s\n", res[i]);
		i++;
	}
	free_strs(res);
	(void)argc;
	(void)argv;
	return 0;
}
