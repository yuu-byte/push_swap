/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_medium_utils.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yuito <yuito@student.42tokyo.jp>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/21 18:40:37 by yuito             #+#    #+#             */
/*   Updated: 2026/08/21 18:40:38 by yuito            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

int	find_max_index(t_stack *s)
{
	int	i;
	int	pos;

	pos = 0;
	i = 1;
	while (i < s->size)
	{
		if (s->data[i] > s->data[pos])
			pos = i;
		i++;
	}
	return (pos);
}

void	rotate_b_to_top(t_ps *ps, int pos)
{
	if (pos <= ps->b.size / 2)
	{
		while (pos > 0)
		{
			ps_op(ps, OP_RB);
			pos--;
		}
	}
	else
	{
		pos = ps->b.size - pos;
		while (pos > 0)
		{
			ps_op(ps, OP_RRB);
			pos--;
		}
	}
}
