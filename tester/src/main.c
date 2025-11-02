#include "insider.h"
#include <stdlib.h>
#include <stdio.h>

int	main(int argc, char **argv, char **envp)
{
	compile_libasm(LIBASM_MANDATORY, envp);
	(void)argc;
	(void)argv;
	return 0;
}
