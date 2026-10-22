#ifndef ERROR_H
#define ERROR_H

typedef enum ErrorType
{
	E_ANY,
	E_INVALID_ARG,
	E_TEST_NOT_FOUND,
	E_LIB_NOT_FOUND,
	E_BROKEN_TEST
} ErrorType_t;

static ErrorType_t errtype = E_ANY;

#define WARNING_COLOR "\033[38;2;244;208;63m"
#define RESET "\033[0m"

void print_err(ErrorType_t type, const char *msg);

#endif
