#include "tester.h"
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
	Tester_t insider = {.fcts_to_test = NULL, .fcts_count = 0};
	return 0;
}

char *name;
int state; // 0 = mandatory  1 = bonus
void *funcptr;