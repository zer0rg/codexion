/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   threads.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgerman- <rgerman-@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:48:27 by rgerman-          #+#    #+#             */
/*   Updated: 2026/05/12 17:58:44 by rgerman-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	init_coders_threads(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->num_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]) != 0)
		{
			sim->stop_simulation = 1;
			while (i > 0)
			{
				i--;
				pthread_join(sim->coders[i].thread, NULL);
			}
			return (-1);
		}
		i++;
	}
}

int	init_monitor_thread(t_simulation *sim, pthread_t *monitor)
{
	int	i;

	if (pthread_create(monitor, NULL, monitor_routine, sim) != 0)
	{
		sim->stop_simulation = 1;
		i = 0;
		while (i < sim->num_coders)
		{
			pthread_join(sim->coders[i].thread, NULL);
			i++;
		}
		return (-1);
	}
}
