#include "tester.h"
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
	if (argc < 2)
	{
		printf("You must provide an argument. Usage: ./libasm_insider "
			   "<mandatory|bonus|all|any function>\n");
		return 1;
	}
	write(1, "\n", 2);
	run_battery(argc, argv, envp);
	return 0;
}
