/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_libasm.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ehosta <ehosta@student.42lyon.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/30 17:08:43 by ehosta            #+#    #+#             */
/*   Updated: 2025/06/30 17:08:44 by ehosta           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_LIBASM_H
# define FT_LIBASM_H

# include <stdlib.h>

/**
 * ASSEMBLY CODE.
 * 
 * Writes 'Hello World' in the standard output.
 */
extern void		print_hello(void);

/**
 * ASSEMBLY CODE.
 * 
 * Takes a string as parameter and returns its length.
 * @param s The string.
 * @returns The total length of the string.
 */
extern size_t	ft_strlen(const char *s);

#endif