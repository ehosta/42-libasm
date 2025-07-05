/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   testing.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 13:39:35 by ehosta            #+#    #+#             */
/*   Updated: 2025/07/02 10:24:45 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TESTING_H
# define TESTING_H

# include <assert.h>
# include <fcntl.h>
# include <stdio.h>
# include <ctype.h>
# include <strings.h>
# include <string.h>
# include <unistd.h>
# include "ft_libasm.h"

# define COLOR_RED "\e[38;2;255;0;0m"
# define COLOR_GREEN "\e[38;2;0;255;0m"
# define COLOR_BLUE "\e[38;2;0;0;255m"
# define COLOR_YELLOW "\e[38;2;255;255;0m"
# define COLOR_RESET "\e[0m"

int		ft_strlen_test(void);
int		print_hello_test(void);

size_t	random_number(size_t min, size_t max);
char	*random_seq(size_t min, size_t max);

#endif /* TESTING_H */