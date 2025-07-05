/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   random_seq.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:42:29 by ehosta            #+#    #+#             */
/*   Updated: 2025/07/01 12:03:49 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

static char*	_fill_seq(char* seq, size_t size, int urand_fd);

size_t	random_number(size_t min, size_t max)
{
	size_t	size;
	int		fd;
	int		read_bytes;
	char	buffer[2];

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return (min);
	while (1)
	{
		bzero(buffer, 2);
		read_bytes = read(fd, buffer, 1);
		if (read_bytes == -1)
		{
			close(fd);
			return min;
		}
		size = (size_t)buffer[0];
		if (size >= min && size <= max)
			break ;
	}
	close(fd);
	return (size);
}

char*	random_seq(size_t min, size_t max)
{
	size_t	seq_size;
	char*	seq;
	int		fd;

	seq_size = random_number(min, max);
	seq = malloc(sizeof(char) * (seq_size + 1));
	if (!seq)
		return (NULL);
	memset(seq, '.', seq_size);
	seq[seq_size] = 0;
	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
	{
		free(seq);
		return (NULL);
	}
	seq = _fill_seq(seq, seq_size, fd);
	close(fd);
	return (seq);
}

static char*	_fill_seq(char* seq, size_t size, int urand_fd)
{
	size_t	i;
	int		read_bytes;
	char	buffer[2];
	char	byte;
	
	i = 0;
	while (i < size)
	{
		bzero(buffer, 2);
		read_bytes = read(urand_fd, buffer, 1);
		if (read_bytes == -1)
			return seq;
		byte = buffer[0];
		if (byte >= 33 && byte <= 126)
		{
			seq[i] = byte;
			i++;
		}
	}
	return (seq);
}
