#ifndef CODEXION_H
# define CODEXION_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <pthread.h>
# include <sys/time.h>

# define FIFO 0
# define EDF 1

# define STATE_THINKING 0
# define STATE_COMPILING 1
# define STATE_DEBUGGING 2
# define STATE_REFACTORING 3
# define STATE_BURNED_OUT 4

typedef struct s_heap_node
{
    int         coder_id;
    long long   priority;
    long long   request_time;
}               t_heap_node;

typedef struct s_priority_queue
{
    t_heap_node *nodes;
    int         size;
    int         capacity;
}               t_priority_queue;

typedef struct s_simulation t_simulation;

typedef struct s_dongle
{
    int                 id;
    pthread_mutex_t     mutex;
    pthread_cond_t      cond;
    int                 available;
    long long           cooldown_end;
    t_priority_queue    *wait_queue;
    t_simulation        *sim;
}               t_dongle;

typedef struct s_coder
{
    int                 id;
    pthread_t           thread;
    long long           last_compile_start;
    long long           deadline;
    int                 compile_count;
    int                 state;
    int                 left_dongle_id;
    int                 right_dongle_id;
    t_simulation        *sim;
}               t_coder;

struct s_simulation
{
    int         num_coders;
    long long   time_to_burnout;
    long long   time_to_compile;
    long long   time_to_debug;
    long long   time_to_refactor;
    int         compile_target;
    long long   dongle_cooldown;
    int         scheduler_type;
    
    t_dongle    *dongles;
    t_coder     *coders;
    
    pthread_mutex_t log_mutex;
    pthread_mutex_t state_mutex;
    pthread_mutex_t start_mutex;
    pthread_cond_t  start_cond;
    int         stop_simulation;
    int         all_threads_ready;
    long long   start_time;
};

long long   get_time_ms(void);
long long   get_elapsed_ms(t_simulation *sim);
int         parse_int_positive(char *str);
int         parse_scheduler(char *str);

void        log_state(t_simulation *sim, int coder_id, const char *message);

t_priority_queue *heap_create(int capacity);
void        heap_destroy(t_priority_queue *heap);
void        heap_insert(t_priority_queue *heap, int coder_id, long long priority, long long request_time);
int         heap_extract_min(t_priority_queue *heap, long long *priority);
int         heap_peek(t_priority_queue *heap);
int         heap_is_empty(t_priority_queue *heap);
void        heap_remove(t_priority_queue *heap, int coder_id);
void        heap_update_priority(t_priority_queue *heap, int coder_id, long long new_priority);

int         dongle_init(t_dongle *dongle, int id, t_simulation *sim);
void        dongle_destroy(t_dongle *dongle);
int         dongle_acquire(t_dongle *dongle, int coder_id, long long deadline);
void        dongle_release(t_dongle *dongle);

void        *coder_routine(void *arg);

void        *monitor_routine(void *arg);

int         simulation_init(t_simulation *sim, int argc, char **argv);
void        simulation_destroy(t_simulation *sim);
int         simulation_run(t_simulation *sim);

#endif
