#include "insider.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv, char **envp)
{
	if (argc ^ 2)
	{
		printf("You must provide an argument. Usage: ./libasm_insider "
			   "<mandatory|bonus|all|any function>\n");
		return 1;
	}
	Insider_t insider = {.functions = NULL};
	return 0;
}
