#include "error.h"
#include "tester.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

static inline __attribute__((always_inline)) bool is_valid(const char *arg)
{
	if (strcmp(arg, "mandatory") == 0 || strcmp(arg, "bonus") == 0 ||
		strcmp(arg, "all") == 0)
		return true;
	for (uint8_t i = 0; i < 12; ++i)
	{
		if (strcmp(arg, functions[i].name) == 0)
			return true;
	}
	return false;
}

static inline __attribute__((always_inline)) void execute_arg(const char *arg,
															  char *const *envp)
{
	CheckType_t type = CUSTOM;

	if (strcmp(arg, "mandatory") == 0)
		type = MANDATORY;
	if (strcmp(arg, "bonus") == 0)
		type = BONUS;
	if (strcmp(arg, "all") == 0)
		type = ALL;

	for (uint8_t i = 0; i < 12; ++i)
	{
		if (type == CUSTOM && strcmp(arg, functions[i].name) != 0)
			continue;
		if (type == MANDATORY && functions[i].is_bonus)
			continue;
		if (type == BONUS && !functions[i].is_bonus)
			continue;
		if (s_has_run & (1 << (functions[i].id + 1)))
			continue;
		s_has_run |= 1 << (functions[i].id + 1);
		int status = compile_test(&functions[i], envp);
		print_err(status, functions[i].name);
	}
}

int run_battery(int argc, char **argv, char *const *envp)
{
	for (int i = 1; i < argc; ++i)
	{
		char **split_input = split(argv[i], " \t\r\n\v");

		for (ssize_t j = 0; split_input[j]; ++j)
		{
			const char *arg = split_input[j];
			if (!is_valid(arg))
			{
				errtype = E_INVALID_ARG;
				return 0;
			}
			execute_arg(arg, envp);
		}
		free_strs(split_input);
	}
	return 1;
}
