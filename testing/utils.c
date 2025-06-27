/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:40:42 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/27 13:40:43 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

void	display_title(void)
{
	printf(COLOR_RED "# ---------------- #" COLOR_RESET "\n");
	printf(COLOR_RED "# LIBASM TESTS     #" COLOR_RESET "\n");
	printf(COLOR_RED "# ---------------- #" COLOR_RESET "\n");
	fflush(stdin);
}
