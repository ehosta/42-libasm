#include "tester.h"
#include <stdio.h>

int main(int argc, char **argv, char **envp)
{
	if (argc < 2)
	{
		printf("You must provide an argument. Usage: ./libasm_insider "
			   "<mandatory|bonus|all|any function>\n");
		return 1;
	}
	int status;
	status = exec("make -C ../ fclean", envp);
	status = exec("mkdir -p .tests/", envp);
	if (status != 0)
		return status;
	run_battery(argc, argv, envp);
	return 0;
}
