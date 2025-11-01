#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>

static inline __attribute__((always_inline))
void	build_ascii_table(const unsigned char *charset, bool table[256])
{
	for (size_t	i = 0; i < 256; ++i)
		table[i] = false;
	for (size_t	i = 0; charset[i]; ++i)
		table[charset[i]] = true;
}

void	free_strs(char **strs)
{
	if (strs == NULL)
		return;
	for (size_t	i = 0; strs[i]; ++i)
		free(strs[i]);
	free(strs);
}

char	**split(const unsigned char *str, const unsigned char *charset)
{
	bool	ascii_table[256];
	build_ascii_table(charset, ascii_table);
	while (*str && ascii_table[(unsigned char)*str])
		str++;
	if (!*str)
		return NULL;
	size_t	word_count = 0;
	size_t	vecsize = 8;
	char	**res = (char **)malloc(sizeof(char *) * (vecsize + 1));
	if (res == NULL)
		return NULL;
	res[0] = NULL;
	while (*str)
	{
		++word_count;
		if (word_count >= vecsize)
			vecsize *= 2;
		char	**tmp = (char **)realloc(res, sizeof(char *) * (vecsize + 1));
		if (tmp == NULL)
		{
			free_strs(res);
			return NULL;
		}
		res = tmp;
		res[word_count] = NULL;
		size_t	new_word_len = 0;
		while (str[new_word_len] && !ascii_table[(unsigned char)str[new_word_len]])
			new_word_len++;
		res[word_count - 1] = (char *)malloc(sizeof(char) * (new_word_len + 1));
		if (res[word_count - 1] == NULL)
		{
			free_strs(res);
			return NULL;
		}
		memcpy(res[word_count - 1], str, new_word_len);
		res[word_count - 1][new_word_len] = 0;
		str += new_word_len;
		while (*str && ascii_table[(unsigned char)*str])
			str++;
	}
	return res;
}
