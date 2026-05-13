/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgerman- <rgerman-@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:43:00 by rgerman-          #+#    #+#             */
/*   Updated: 2026/05/12 17:43:01 by rgerman-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int argc, char **argv)
{
	t_simulation	sim;

	if (simulation_init(&sim, argc, argv) != 0)
		return (1);
	if (simulation_run(&sim) != 0)
	{
		simulation_destroy(&sim);
		return (1);
	}
	simulation_destroy(&sim);
	return (0);
}
