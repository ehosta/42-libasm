/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 16:50:27 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/30 17:32:21 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

int	ft_strlen_test(void)
{
	const char	s[] = "Ceci est un test";

	printf("'%s': %zu\n", s, ft_strlen(s));
	return (1);
}
