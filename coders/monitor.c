#include "codexion.h"

void *monitor_routine(void *arg)
{
    t_simulation *sim;
    int i;
    long long current_time;
    int all_done;
    
    sim = (t_simulation *)arg;
    
    pthread_mutex_lock(&sim->start_mutex);
    sim->all_threads_ready++;
    pthread_cond_wait(&sim->start_cond, &sim->start_mutex);
    pthread_mutex_unlock(&sim->start_mutex);
    
    while (1)
    {
        usleep(500);
        
        pthread_mutex_lock(&sim->state_mutex);
        if (sim->stop_simulation)
        {
            pthread_mutex_unlock(&sim->state_mutex);
            break;
        }
        
        all_done = 1;
        current_time = get_time_ms();
        
        i = 0;
        while (i < sim->num_coders)
        {
            if (sim->coders[i].compile_count < sim->compile_target)
                all_done = 0;
            
            if (sim->coders[i].compile_count < sim->compile_target &&
                sim->coders[i].state != STATE_COMPILING && 
                sim->coders[i].state != STATE_BURNED_OUT)
            {
                if (current_time > sim->coders[i].deadline)
                {
                    sim->coders[i].state = STATE_BURNED_OUT;
                    sim->stop_simulation = 1;
                    pthread_mutex_unlock(&sim->state_mutex);
                    
                    log_state(sim, sim->coders[i].id, "burned out");
                    
                    i = 0;
                    while (i < sim->num_coders)
                    {
                        pthread_cond_broadcast(&sim->dongles[i].cond);
                        i++;
                    }
                    
                    return (NULL);
                }
            }
            i++;
        }
        
        if (all_done)
        {
            sim->stop_simulation = 1;
            pthread_mutex_unlock(&sim->state_mutex);
            break;
        }
        
        pthread_mutex_unlock(&sim->state_mutex);
    }
    
    return (NULL);
}
