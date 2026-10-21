#define _POSIX_C_SOURCE 200809L
#include "tester.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static inline __attribute__((always_inline)) char *
join_with_sep(const char *path, const char *binary)
{
	char *path_sep = join(path, "/");
	char *path_sep_bin = join(path_sep, binary);
	if (path_sep != NULL)
		free(path_sep);
	return path_sep_bin;
}

static inline __attribute__((always_inline)) void
execute_from_path(const char *binary, char *const *argv, char *const *envp)
{
	if (binary == NULL)
		return;
	execve(binary, argv, envp);
	const char *path = getenv("PATH");
	char **splitted_paths = NULL;
	if (path != NULL)
		splitted_paths = split(path, ":");
	for (size_t i = 0; splitted_paths[i]; ++i)
	{
		char *full = join_with_sep(splitted_paths[i], binary);
		if (access(full, 0) != 0)
		{
			free(full);
			continue;
		}
		execve(full, argv, envp);
	}
	free_strs(splitted_paths);
}

static inline __attribute__((always_inline)) int
exec_custom(const char *binary, char *const *argv, char *const *envp)
{
	int pipe_fd[2];
	if (pipe(pipe_fd) == -1)
		return -1;
	pid_t pid = fork();
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
			exit(-1);
		execute_from_path(binary, argv, envp);
		exit(-1);
	}
	close(pipe_fd[1]);
	char buffer[1024];
	int n;
	while ((n = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
		write(STDOUT_FILENO, buffer, n);
	close(pipe_fd[0]);
	int status = 0;
	waitpid(pid, &status, 0);
	return status;
}

int exec(const char *cmd, char *const *envp)
{
	int status;

	char **av = split(cmd, " ");
	status = exec_custom(av[0], (char *const *)av, envp);
	free_strs(av);
	return status;
}

static inline __attribute__((always_inline)) void
load_linking_command(const Function_t *fct, char *const *envp)
{
	char *libname = NULL;
	if (!fct->is_bonus)
		libname = strdup(" ../libasm.a");
	else
		libname = strdup(" ../libasm_bonus.a");

	char *function_file = join(fct->name, ".c");
	char *testname = join_with_sep("unit", function_file);

	char *clang_lib = join(COMPILATION_FLAGS, testname);
	char *clang_lib_test = join(clang_lib, libname);
	char *clang_output = join(" -o ./.tests/test_", fct->name);
	char *final = join(clang_lib_test, clang_output);

	if (libname != NULL)
		free(libname);

	if (function_file != NULL)
		free(function_file);
	if (testname != NULL)
		free(testname);

	if (clang_lib != NULL)
		free(clang_lib);
	if (clang_lib_test != NULL)
		free(clang_lib_test);
	if (clang_output != NULL)
		free(clang_output);

	exec(final, envp);
	free(final);

	char *run_test = join("./.tests/test_", fct->name);
	exec(run_test, envp);
	free(run_test);
}

int compile_test(const Function_t *fct, char *const *envp)
{
	int status;
	if (fct->is_bonus && !bonus_enabled)
	{
		bonus_enabled = true;
		status = exec("make -C ../ bonus", envp);
	}
	if (!fct->is_bonus && !mandatory_enabled)
	{
		bonus_enabled = true;
		status = exec("make -C ../ all", envp);
	}
	if (status != 0)
		return status;

	load_linking_command(fct, envp);
	return status;
}
