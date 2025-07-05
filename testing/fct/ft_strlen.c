/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 16:50:27 by ehosta            #+#    #+#             */
/*   Updated: 2025/07/02 10:29:08 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

int	ft_strlen_test(void)
{
	size_t	i;
	char*	s;
	size_t	len;
	size_t	lib_len;

	i = -1;
	while (++i < 20)
	{
		s = random_seq(10, 32);
		if (!s)
			return (1);
		lib_len = strlen(s);
		printf("Test %zu\t" COLOR_GREEN "%zu" COLOR_YELLOW "\t%s" COLOR_RESET "\n", ft_strlen(s), s);
		free(s);
	}
	return (1);
}
