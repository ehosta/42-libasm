#define _POSIX_C_SOURCE 200809L
#include "insider.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

static inline __attribute__((always_inline))
char	*join_with_sep(const char *path, const char *binary)
{
	char	*path_sep = join(path, "/");
	char	*path_sep_bin = join(path_sep, binary);
	if (path_sep != NULL)
		free(path_sep);
	return path_sep_bin;
}

static inline __attribute__((always_inline))
char	*load_linking_command(ProjectType_t type)
{
	char	*libname = NULL, *testname = NULL;
	if (type == LIBASM_MANDATORY)
	{
		testname = strdup("unit/mandatory.c");
		libname = strdup(" ../libasm.a");
	}
	else
	{
		testname = strdup("unit/bonus.c");
		libname = strdup(" ../libasm_bonus.a");
	}
	char	*clang_lib = join(COMPILATION_FLAGS, testname);
	char	*clang_lib_test = join(clang_lib, libname);
	char	*final = join(clang_lib_test, " -o unit_test");
	if (clang_lib != NULL)
		free(clang_lib);
	if (clang_lib_test != NULL)
		free(clang_lib_test);
	if (testname != NULL)
		free(testname);
	if (libname != NULL)
		free(libname);
	return final;
}

static inline __attribute__((always_inline))
void	execute_from_path(const char *binary, char *const *argv, char *const *envp)
{
	if (binary == NULL)
		return;
	execve(binary, argv, envp);
	const char	*path = getenv("PATH");
	char		**splitted_paths = NULL;
	if (path != NULL)
		splitted_paths = split(path, ":");
	for (size_t	i = 0; splitted_paths[i]; ++i)
	{
		char	*full = join_with_sep(splitted_paths[i], binary);
		if (access(full, 0) != 0)
		{
			free(full);
			continue;
		}
		execve(full, argv, envp);
	}
	free_strs(splitted_paths);
}

static inline __attribute__((always_inline))
int	exec(const char *binary, char *const *argv, char *const *envp)
{
	int	pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		return -1;
	pid_t	pid = fork();
	if (pid == -1)
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
		return -1;
	}
	if (pid == 0)
	{
		close(pipe_fd[0]);
		int dupfd = dup2(pipe_fd[1], STDOUT_FILENO);
		close(pipe_fd[1]);
		if (dupfd == -1)
			return -1;
		execute_from_path(binary, argv, envp);
		return -1;
	}
	close(pipe_fd[1]);
	char	buffer[1024];
	int	n;
	while ((n = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
		write(STDOUT_FILENO, buffer, n);
	close(pipe_fd[0]);
	int	status = 0;
	waitpid(pid, &status, 0);
	return status;
}

int	compile_test(ProjectType_t type, char *const *envp)
{
	int		status;
	char	**av = split("make -C ../ fclean", " ");
	status = exec("make", (char *const *)av, envp);
	free_strs(av);
	if (status != 0)
		return status;
	if (type == LIBASM_MANDATORY)
		av = split("make -C ../ all", " ");
	else
		av = split("make -C ../ bonus", " ");
	status = exec("make", (char *const *)av, envp);
	free_strs(av);
	if (status != 0)
		return status;
	char	*linking_cmd = load_linking_command(type);
	av = split(linking_cmd, " ");
	status = exec("clang", (char *const *)av, envp);
	free_strs(av);
	free(linking_cmd);
	return status;
}

int	run_test(char *const *envp)
{
	int		status;
	char	**av = split("./unit_test", " ");
	status = exec("./unit_test", (char *const *)av, envp);
	free_strs(av);
	return status;
}
