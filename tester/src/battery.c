#include "tester.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

static uint16_t s_has_run = 0;

static inline __attribute__((always_inline)) bool is_valid(const char *arg)
{
	if (strcmp(arg, "mandatory") == 0 || strcmp(arg, "bonus") == 0)
		return true;
	for (uint8_t i = 0; i < 12; ++i)
	{
		if (strcmp(arg, functions[i].name) == 0)
			return true;
	}
	return false;
}

int run_battery(const char *input)
{
	const char **split_input = split(input, " \t\r\n\v");

	for (ssize_t i = 0; split_input[i]; ++i)
	{
		const char *arg = split_input[i];
		if (!is_valid(arg))
		{
			
			return 1;
		}
	}
}
