#include "codexion.h"

static int should_stop(t_simulation *sim)
{
    int stop;
    
    pthread_mutex_lock(&sim->state_mutex);
    stop = sim->stop_simulation;
    pthread_mutex_unlock(&sim->state_mutex);
    return (stop);
}

static int try_acquire_dongles(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;
    int first_id;
    int second_id;
    long long deadline;
    
    first_id = coder->left_dongle_id;
    second_id = coder->right_dongle_id;
    
    deadline = coder->deadline;
    
    if (first_id == second_id)
    {
        if (!dongle_acquire(&coder->sim->dongles[first_id], coder->id, deadline))
            return (0);
        if (should_stop(coder->sim))
        {
            dongle_release(&coder->sim->dongles[first_id]);
            return (0);
        }
        log_state(coder->sim, coder->id, "has taken a dongle");
        log_state(coder->sim, coder->id, "has taken a dongle");
        return (1);
    }
    
    if (first_id < second_id)
    {
        first = &coder->sim->dongles[first_id];
        second = &coder->sim->dongles[second_id];
    }
    else
    {
        first = &coder->sim->dongles[second_id];
        second = &coder->sim->dongles[first_id];
    }
    
    if (!dongle_acquire(first, coder->id, deadline))
        return (0);
    
    if (should_stop(coder->sim))
    {
        dongle_release(first);
        return (0);
    }
    
    log_state(coder->sim, coder->id, "has taken a dongle");
    
    if (!dongle_acquire(second, coder->id, deadline))
    {
        dongle_release(first);
        return (0);
    }
    
    if (should_stop(coder->sim))
    {
        dongle_release(second);
        dongle_release(first);
        return (0);
    }
    
    log_state(coder->sim, coder->id, "has taken a dongle");
    
    return (1);
}

static void release_dongles(t_coder *coder)
{
    t_dongle *first;
    t_dongle *second;
    int first_id;
    int second_id;
    
    first_id = coder->left_dongle_id;
    second_id = coder->right_dongle_id;
    
    if (first_id == second_id)
    {
        dongle_release(&coder->sim->dongles[first_id]);
        return;
    }
    
    if (first_id < second_id)
    {
        first = &coder->sim->dongles[first_id];
        second = &coder->sim->dongles[second_id];
    }
    else
    {
        first = &coder->sim->dongles[second_id];
        second = &coder->sim->dongles[first_id];
    }
    
    dongle_release(first);
    dongle_release(second);
}

void *coder_routine(void *arg)
{
    t_coder *coder;
    
    coder = (t_coder *)arg;
    
    pthread_mutex_lock(&coder->sim->start_mutex);
    coder->sim->all_threads_ready++;
    pthread_cond_wait(&coder->sim->start_cond, &coder->sim->start_mutex);
    pthread_mutex_unlock(&coder->sim->start_mutex);
    
    pthread_mutex_lock(&coder->sim->state_mutex);
    coder->deadline = coder->sim->start_time + coder->sim->time_to_burnout;
    pthread_mutex_unlock(&coder->sim->state_mutex);
    
    while (1)
    {
        pthread_mutex_lock(&coder->sim->state_mutex);
        if (coder->sim->stop_simulation || coder->compile_count >= coder->sim->compile_target)
        {
            pthread_mutex_unlock(&coder->sim->state_mutex);
            break;
        }
        pthread_mutex_unlock(&coder->sim->state_mutex);
        
        if (!try_acquire_dongles(coder))
            break;
        
        pthread_mutex_lock(&coder->sim->state_mutex);
        coder->state = STATE_COMPILING;
        coder->last_compile_start = get_time_ms();
        coder->deadline = coder->last_compile_start + coder->sim->time_to_burnout;
        pthread_mutex_unlock(&coder->sim->state_mutex);
        
        log_state(coder->sim, coder->id, "is compiling");
        usleep(coder->sim->time_to_compile * 1000);
        
        release_dongles(coder);
        
        pthread_mutex_lock(&coder->sim->state_mutex);
        if (coder->sim->stop_simulation)
        {
            pthread_mutex_unlock(&coder->sim->state_mutex);
            break;
        }
        coder->state = STATE_DEBUGGING;
        pthread_mutex_unlock(&coder->sim->state_mutex);
        
        log_state(coder->sim, coder->id, "is debugging");
        usleep(coder->sim->time_to_debug * 1000);
        
        pthread_mutex_lock(&coder->sim->state_mutex);
        if (coder->sim->stop_simulation)
        {
            pthread_mutex_unlock(&coder->sim->state_mutex);
            break;
        }
        coder->state = STATE_REFACTORING;
        pthread_mutex_unlock(&coder->sim->state_mutex);
        
        log_state(coder->sim, coder->id, "is refactoring");
        usleep(coder->sim->time_to_refactor * 1000);
        
        pthread_mutex_lock(&coder->sim->state_mutex);
        coder->compile_count++;
        coder->state = STATE_THINKING;
        pthread_mutex_unlock(&coder->sim->state_mutex);
    }
    
    return (NULL);
}
