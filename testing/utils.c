/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:40:42 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/27 13:56:25 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

void	display_title(void)
{
	printf(COLOR_RED "# --------------------------------- #" COLOR_RESET "\n");
	printf(COLOR_RED "#           LIBASM TESTS            #" COLOR_RESET "\n");
	printf(COLOR_RED "# --------------------------------- #" COLOR_RESET "\n");
	fflush(stdin);
}

void	display_subtitle(const char *str)
{
	printf(COLOR_BLUE "\n(+) %s" COLOR_RESET "\n\n", str);
	fflush(stdin);
}
