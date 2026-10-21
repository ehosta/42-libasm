#ifndef INSIDER_H
#define INSIDER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define COMPILATION_FLAGS "clang -Wall -Wextra -Werror -Wpedantic -Wshadow -g3 "

typedef enum FunctionID
{
	FT_STRLEN,
	FT_STRCPY,
	FT_STRCMP,
	FT_WRITE,
	FT_READ,
	FT_STRDUP,
	FT_CALLOC,
	FT_ATOI_BASE,
	FT_LIST_PUSH_FRONT,
	FT_LIST_SIZE,
	FT_LIST_SORT,
	FT_SIMD_MEMCHR
} FunctionID_t;

typedef struct Function
{
	const char *name;
	bool is_bonus;
	FunctionID_t id;
} Function_t;

static const Function_t functions[] = {
	// Mandatory part
	{.name = "ft_strlen", .is_bonus = false, .id = FT_STRLEN},
	{.name = "ft_strcpy", .is_bonus = false, .id = FT_STRCPY},
	{.name = "ft_strcmp", .is_bonus = false, .id = FT_STRCMP},
	{.name = "ft_write", .is_bonus = false, .id = FT_WRITE},
	{.name = "ft_read", .is_bonus = false, .id = FT_READ},
	{.name = "ft_strdup", .is_bonus = false, .id = FT_STRDUP},
	{.name = "ft_calloc", .is_bonus = false, .id = FT_CALLOC},
	// Bonus part
	{.name = "ft_atoi_base", .is_bonus = true, .id = FT_ATOI_BASE},
	{.name = "ft_list_push_front", .is_bonus = true, .id = FT_LIST_PUSH_FRONT},
	{.name = "ft_list_size", .is_bonus = true, .id = FT_LIST_SIZE},
	{.name = "ft_list_sort", .is_bonus = true, .id = FT_LIST_SORT},
	{.name = "ft_simd_memchr", .is_bonus = true, .id = FT_SIMD_MEMCHR}};

typedef enum CheckType
{
	MANDATORY,
	BONUS,
	CUSTOM,
	ALL
} CheckType_t;

static bool mandatory_enabled = false;
static bool bonus_enabled = false;

void free_strs(char **strs);
char **split(const char *str, const char *charset);
char *join(const char *s1, const char *s2);

int run_battery(int argc, char **argv, char *const *envp);

int exec(const char *cmd, char *const *envp);
int compile_test(const Function_t *fct, char *const *envp);
int run_test(char *const *envp);

#endif
