#ifndef INSIDER_H
#define INSIDER_H

#include "functions.h"
#include <stdlib.h>

#define COMPILATION_FLAGS "clang -Wall -Wextra -Werror -Wpedantic -Wshadow -g3 "

static const char *mandatory_functions[] = {
	"ft_strlen", "ft_strcpy", "ft_strcmp", "ft_write",
	"ft_read",	 "ft_strdup", "ft_calloc", NULL};

static const char *bonus_functions[] = {"ft_atoi_base",
										"ft_list_push_front",
										"ft_list_size",
										"ft_list_sort",
										"ft_list_remove_if",
										"ft_simd_memchr",
										NULL};

typedef struct Insider
{
	char *functions;
} Insider_t;

void free_strs(char **strs);
char **split(const char *str, const char *charset);
char *join(const char *s1, const char *s2);
int compile_test(ProjectType_t type, char *const *envp);
int run_test(char *const *envp);

#endif
