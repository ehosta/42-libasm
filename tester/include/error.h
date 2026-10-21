#ifndef ERROR_H
#define ERROR_H

typedef enum ErrorType
{
	E_ANY,
	E_INVALID_ARG,
} ErrorType_t;

static ErrorType_t errtype = E_ANY;

#endif
