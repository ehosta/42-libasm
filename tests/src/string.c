#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include <unistd.h>

typedef struct Word
{
	char		*content;
	struct Word	*next;
}	Word_t;

typedef struct Split
{
	size_t	word_count;
	Word_t	*words;
}	Split_t;

static inline __attribute__((always_inline))
void	free_split_(Word_t *words)
{
	if (words == NULL)
		return ;
	Word_t	*node = words;
	while (node)
	{
		Word_t	*tmp = node->next;
		free(node);
		node = tmp;
	}
}

static inline __attribute__((always_inline))
void	free_split_content_(Word_t *words)
{
	if (words == NULL)
		return ;
	Word_t	*node = words;
	while (node)
	{
		if (node->content)
			free(node->content);
		Word_t	*tmp = node->next;
		free(node);
		node = tmp;
	}
}

static inline __attribute__((always_inline))
bool	is_in_charset_(char c, const char *charset)
{
	while (*charset)
	{
		if (c == *charset)
			return true;
		++charset;
	}
	return false;
}

static inline __attribute__((always_inline))
size_t	strlen_until_(const char *str, const char *charset)
{
	if (charset[0] == 0)
		charset = "\0";
	if (charset[1] == 0)
	{
		size_t	len = 0;
		while (str[len] && str[len] != charset[0])
			++len;
		return len;
	}
	size_t	len = 0;
	while (str[len] && !is_in_charset_(str[len], charset))
		len++;
	return len;
}

char	**split(const char *str, const char *charset)
{
	Split_t	data = {
		.word_count = 0,
		.words = NULL,
	};
	while (*str)
	{
		while (*str && is_in_charset_(*str, charset))
			str++;
		if (!*str)
			break;
		data.word_count++;
		size_t	word_len = strlen_until_(str, charset);
		Word_t	*w = malloc(sizeof(Word_t));
		if (w == NULL)
		{
			free_split_content_(data.words);
			return NULL;
		}
		w->content = malloc(sizeof(char) * (word_len + 1));
		w->next = NULL;
		if (w->content == NULL)
		{
			free_split_content_(data.words);
			return NULL;
		}
		strncpy(w->content, str, word_len);
		w->content[word_len] = 0;
		if (data.words == NULL)
			data.words = w;
		else
		{
			Word_t	*last = data.words;
			while (last->next)
				last = last->next;
			last->next = w;
		}
		while (*str && !is_in_charset_(*str, charset))
			str++;
	}
	if (!data.word_count)
		return NULL;
	char	**res = malloc(sizeof(char **) * (data.word_count + 1));
	if (res == NULL)
	{
		free_split_content_(data.words);
		return NULL;
	}
	size_t	i = 0;
	Word_t	*elt = data.words;
	for (; i < data.word_count; ++i)
	{
		res[i] = elt->content;
		elt = elt->next;
	}
	res[i] = NULL;
	free_split_(data.words);
	return res;
}
