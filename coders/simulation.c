#include "codexion.h"

static int validate_args(t_simulation *sim, int argc, char **argv)
{
    if (argc != 9)
    {
        fprintf(stderr, "Usage: %s number_of_coders time_to_burnout time_to_compile time_to_debug \
            time_to_refactor number_of_compiles_required dongle_cooldown scheduler\n", argv[0]);
        return (-1);
    }
    
    sim->num_coders = parse_int_positive(argv[1]);
    sim->time_to_burnout = parse_int_positive(argv[2]);
    sim->time_to_compile = parse_int_positive(argv[3]);
    sim->time_to_debug = parse_int_positive(argv[4]);
    sim->time_to_refactor = parse_int_positive(argv[5]);
    sim->compile_target = parse_int_positive(argv[6]);
    sim->dongle_cooldown = parse_int_positive(argv[7]);
    sim->scheduler_type = parse_scheduler(argv[8]);
    
    if (sim->num_coders <= 0 || sim->time_to_burnout <= 0 || 
        sim->time_to_compile <= 0 || sim->time_to_debug <= 0 ||
        sim->time_to_refactor <= 0 || sim->compile_target <= 0 ||
        sim->dongle_cooldown < 0 || sim->scheduler_type < 0)
    {
        fprintf(stderr, "Error: Invalid arguments\n");
        return (-1);
    }
    
    return (0);
}

static int init_mutexes(t_simulation *sim)
{
    if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
        return (-1);
    if (pthread_mutex_init(&sim->state_mutex, NULL) != 0)
    {
        pthread_mutex_destroy(&sim->log_mutex);
        return (-1);
    }
    if (pthread_mutex_init(&sim->start_mutex, NULL) != 0)
    {
        pthread_mutex_destroy(&sim->log_mutex);
        pthread_mutex_destroy(&sim->state_mutex);
        return (-1);
    }
    if (pthread_cond_init(&sim->start_cond, NULL) != 0)
    {
        pthread_mutex_destroy(&sim->log_mutex);
        pthread_mutex_destroy(&sim->state_mutex);
        pthread_mutex_destroy(&sim->start_mutex);
        return (-1);
    }
    return (0);
}

static int init_dongles(t_simulation *sim)
{
    int i;
    int num_dongles;
    
    num_dongles = sim->num_coders;
    sim->dongles = malloc(sizeof(t_dongle) * num_dongles);
    if (!sim->dongles)
        return (-1);
    
    i = 0;
    while (i < num_dongles)
    {
        if (dongle_init(&sim->dongles[i], i, sim) != 0)
        {
            while (i > 0)
            {
                i--;
                dongle_destroy(&sim->dongles[i]);
            }
            free(sim->dongles);
            sim->dongles = NULL;
            return (-1);
        }
        i++;
    }
    return (0);
}

static int init_coders(t_simulation *sim)
{
    int i;
    
    sim->coders = malloc(sizeof(t_coder) * sim->num_coders);
    if (!sim->coders)
        return (-1);
    
    i = 0;
    while (i < sim->num_coders)
    {
        sim->coders[i].id = i + 1;
        sim->coders[i].compile_count = 0;
        sim->coders[i].state = STATE_THINKING;
        sim->coders[i].last_compile_start = 0;
        sim->coders[i].deadline = 0;
        sim->coders[i].sim = sim;
        sim->coders[i].left_dongle_id = i;
        sim->coders[i].right_dongle_id = (i + 1) % sim->num_coders;
        i++;
    }
    return (0);
}

int simulation_init(t_simulation *sim, int argc, char **argv)
{
    memset(sim, 0, sizeof(t_simulation));
    
    if (validate_args(sim, argc, argv) != 0)
        return (-1);
    
    if (init_mutexes(sim) != 0)
        return (-1);
    
    if (init_dongles(sim) != 0)
    {
        pthread_mutex_destroy(&sim->log_mutex);
        pthread_mutex_destroy(&sim->state_mutex);
        pthread_mutex_destroy(&sim->start_mutex);
        pthread_cond_destroy(&sim->start_cond);
        return (-1);
    }
    
    if (init_coders(sim) != 0)
    {
        simulation_destroy(sim);
        return (-1);
    }
    
    return (0);
}

void simulation_destroy(t_simulation *sim)
{
    int i;
    
    if (sim->dongles)
    {
        i = 0;
        while (i < sim->num_coders)
        {
            dongle_destroy(&sim->dongles[i]);
            i++;
        }
        free(sim->dongles);
    }
    
    if (sim->coders)
        free(sim->coders);
    
    pthread_mutex_destroy(&sim->log_mutex);
    pthread_mutex_destroy(&sim->state_mutex);
    pthread_mutex_destroy(&sim->start_mutex);
    pthread_cond_destroy(&sim->start_cond);
}

int simulation_run(t_simulation *sim)
{
    pthread_t monitor;
    int i;
    
    sim->all_threads_ready = 0;
    sim->stop_simulation = 0;
    
    i = 0;
    while (i < sim->num_coders)
    {
        if (pthread_create(&sim->coders[i].thread, NULL, coder_routine, &sim->coders[i]) != 0)
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
    
    if (pthread_create(&monitor, NULL, monitor_routine, sim) != 0)
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
    
    while (sim->all_threads_ready < sim->num_coders + 1)
        usleep(100);
    
    sim->start_time = get_time_ms();
    
    pthread_mutex_lock(&sim->start_mutex);
    pthread_cond_broadcast(&sim->start_cond);
    pthread_mutex_unlock(&sim->start_mutex);
    
    i = 0;
    while (i < sim->num_coders)
    {
        pthread_join(sim->coders[i].thread, NULL);
        i++;
    }
    
    pthread_join(monitor, NULL);
    
    return (0);
}
