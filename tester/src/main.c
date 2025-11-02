#include "insider.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int	main(int argc, char **argv, char **envp)
{
	if (argc ^ 2)
	{
		printf("You must provide an argument. Usage: ./libasm_insider <mandatory|bonus>\n");
		return 1;
	}
	ProjectType_t	type;
	if (strcmp("mandatory", argv[1]) == 0)
		type = LIBASM_MANDATORY;
	else if (strcmp("bonus", argv[1]) == 0)
		type = LIBASM_BONUS;
	else
	{
		printf("You must provide an argument. Usage: ./libasm_insider <mandatory|bonus>\n");
		return 1;
	}
	compile_test(type, envp);
	(void)argc;
	(void)argv;
	return 0;
}
