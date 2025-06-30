/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:39:31 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/30 17:48:25 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "testing.h"

static void	_display_title(void)
{
	printf(COLOR_RED "# --------------------------------- #" COLOR_RESET "\n");
	printf(COLOR_RED "#           LIBASM TESTS            #" COLOR_RESET "\n");
	printf(COLOR_RED "# --------------------------------- #" COLOR_RESET "\n");
	fflush(stdout);
}

static void	_display_subtitle(const char *str)
{
	printf(COLOR_BLUE "\n(+) %s" COLOR_RESET "\n\n", str);
	fflush(stdout);
}

int	main(void)
{
	_display_title();
	_display_subtitle("Fct: print_hello.s");
	assert(print_hello_test());
	_display_subtitle("Fct: ft_strlen.s");
	assert(ft_strlen_test());
}
