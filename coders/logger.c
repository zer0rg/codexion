#include "codexion.h"

void log_state(t_simulation *sim, int coder_id, const char *message)
{
    long long timestamp;
    
    pthread_mutex_lock(&sim->log_mutex);
    timestamp = get_elapsed_ms(sim);
    printf("%lld %d %s\n", timestamp, coder_id, message);
    pthread_mutex_unlock(&sim->log_mutex);
}
