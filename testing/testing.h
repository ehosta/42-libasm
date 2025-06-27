/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:39:35 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/27 13:55:15 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTING_H
# define TESTING_H

# include <assert.h>
# include <stdio.h>

# define COLOR_RED "\e[38;2;255;0;0m"
# define COLOR_BLUE "\e[38;2;0;0;255m"
# define COLOR_RESET "\e[0m"

void		display_title(void);
void		display_subtitle(const char *str);
int			print_hello_test(void);

/**
 * ASSEMBLY CODE.
 * 
 * Writes 'Hello World' in the standard output.
 */
extern void	print_hello(void);

#endif /* TESTING_H */