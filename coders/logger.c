/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   logger.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgerman- <rgerman-@student.42madrid.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:43:17 by rgerman-          #+#    #+#             */
/*   Updated: 2026/05/12 17:43:18 by rgerman-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	log_state(t_simulation *sim, int coder_id, const char *message)
{
	long long	timestamp;

	pthread_mutex_lock(&sim->log_mutex);
	timestamp = get_elapsed_ms(sim);
	printf("%lld %d %s\n", timestamp, coder_id, message);
	pthread_mutex_unlock(&sim->log_mutex);
}
