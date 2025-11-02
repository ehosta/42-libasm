#ifndef INSIDER_H
#define INSIDER_H

#define COMPILATION_FLAGS "-Wall -Wextra -Werror -Wpedantic -Wshadow -g3"

typedef enum ProjectType
{
	LIBASM_MANDATORY,
	LIBASM_BONUS
}	ProjectType_t;

void	free_strs(char **strs);
char	**split(const char *str, const char *charset);
char	*join(const char *s1, const char *s2);
int		compile_libasm(ProjectType_t type, char *const *envp);

#endif
