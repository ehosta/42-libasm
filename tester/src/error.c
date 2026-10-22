#include "error.h"
#include <stddef.h>
#include <string.h>
#include <unistd.h>

void print_err(ErrorType_t type, const char *msg)
{
	if (type == E_ANY)
	{
		write(2, "\n", 2);
		return;
	}

	write(2, WARNING_COLOR "[", 20);
	if (msg)
	{
		size_t msglen = strlen(msg);
		if (msglen)
			write(2, msg, msglen);
	}
	else
		write(2, "error", 6);

	write(2, ": ", 3);
	switch (type)
	{
	case E_INVALID_ARG:
		write(2, "invalid arg", 12);
		break;
	case E_TEST_NOT_FOUND:
		write(2, "test not found", 15);
		break;
	case E_LIB_NOT_FOUND:
		write(2, "lib not found", 14);
		break;
	case E_BROKEN_TEST:
		write(2, "broken test", 12);
		break;
	default:
		break;
	}
	write(2, "]" RESET "\n", 7);
}
