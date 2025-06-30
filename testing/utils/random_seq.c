/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   random_seq.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:42:29 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/30 18:03:25 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

static size_t	_get_size(size_t min, size_t max)
{
	size_t	size;
	int		fd;
	int		read_bytes;
	char	buffer[2];

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return (min);
	bzero(buffer, 2);
	read_bytes = read(fd, buffer, 1);
	if (read_bytes == -1)
	{
		close(fd);
		return min;
	}
	size = (size_t)buffer[0];
	if (size < min)
		size = min;
	else if (size > max)
		size = max;
	close(fd);
	return (size);
}

char	*random_seq(size_t min, size_t max)
{
	size_t	seq_size;
	char	*seq;

	return (seq);
}
