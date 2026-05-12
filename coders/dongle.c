#include "codexion.h"

int dongle_init(t_dongle *dongle, int id, t_simulation *sim)
{
    dongle->id = id;
    dongle->sim = sim;
    dongle->available = 1;
    dongle->cooldown_end = 0;
    dongle->wait_queue = heap_create(sim->num_coders);
    if (!dongle->wait_queue)
        return (-1);
    if (pthread_mutex_init(&dongle->mutex, NULL) != 0)
    {
        heap_destroy(dongle->wait_queue);
        return (-1);
    }
    if (pthread_cond_init(&dongle->cond, NULL) != 0)
    {
        pthread_mutex_destroy(&dongle->mutex);
        heap_destroy(dongle->wait_queue);
        return (-1);
    }
    return (0);
}

void dongle_destroy(t_dongle *dongle)
{
    pthread_mutex_destroy(&dongle->mutex);
    pthread_cond_destroy(&dongle->cond);
    heap_destroy(dongle->wait_queue);
}

int dongle_acquire(t_dongle *dongle, int coder_id, long long deadline)
{
    long long priority;
    long long current_time;
    int waiter;
    int is_my_turn;
    long long wait_priority;
    struct timespec ts;
    long long wait_until;
    
    pthread_mutex_lock(&dongle->mutex);
    
    priority = deadline;
    if (dongle->sim->scheduler_type == FIFO)
        priority = get_time_ms();
    
    current_time = get_time_ms();
    if (!dongle->available || current_time < dongle->cooldown_end)
    {
        heap_insert(dongle->wait_queue, coder_id, priority, get_time_ms());
        is_my_turn = 0;
        while (!is_my_turn)
        {
            waiter = heap_peek(dongle->wait_queue);
            if (waiter == coder_id)
            {
                current_time = get_time_ms();
                if (dongle->available && current_time >= dongle->cooldown_end)
                {
                    is_my_turn = 1;
                    break;
                }
                else if (dongle->available && current_time < dongle->cooldown_end)
                {
                    wait_until = dongle->cooldown_end + 1;
                    ts.tv_sec = wait_until / 1000;
                    ts.tv_nsec = (wait_until % 1000) * 1000000;
                    pthread_cond_timedwait(&dongle->cond, &dongle->mutex, &ts);
                    continue;
                }
            }
            if (dongle->sim->stop_simulation)
            {
                heap_remove(dongle->wait_queue, coder_id);
                pthread_mutex_unlock(&dongle->mutex);
                return (0);
            }
            pthread_cond_wait(&dongle->cond, &dongle->mutex);
        }
        heap_extract_min(dongle->wait_queue, &wait_priority);
    }
    
    dongle->available = 0;
    pthread_mutex_unlock(&dongle->mutex);
    
    return (1);
}

void dongle_release(t_dongle *dongle)
{
    pthread_mutex_lock(&dongle->mutex);
    
    dongle->available = 1;
    dongle->cooldown_end = get_time_ms() + dongle->sim->dongle_cooldown;
    
    pthread_cond_broadcast(&dongle->cond);
    pthread_mutex_unlock(&dongle->mutex);
}
