/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_complex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yuito <yuito@student.42tokyo.jp>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 18:40:30 by yuito             #+#    #+#             */
/*   Updated: 2026/08/21 18:40:32 by yuito            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

static int	count_bits(int n)
{
	int	b;

	b = 0;
	while (((n - 1) >> b) != 0)
		b++;
	return (b);
}

static void	radix_one_pass(t_ps *ps, int bit)
{
	int	i;
	int	sz;

	sz = ps->a.size;
	i = 0;
	while (i < sz)
	{
		if (((ps->a.data[0] >> bit) & 1) == 1)
			ps_op(ps, OP_RA);
		else
			ps_op(ps, OP_PB);
		i++;
	}
	while (ps->b.size > 0)
		ps_op(ps, OP_PA);
}

void	sort_complex(t_ps *ps)
{
	int	bits;
	int	bit;

	bits = count_bits(ps->a.size);
	bit = 0;
	while (bit < bits)
	{
		if (ps_is_sorted(&ps->a))
			return ;
		radix_one_pass(ps, bit);
		bit++;
	}
}
