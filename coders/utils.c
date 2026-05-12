#include "codexion.h"

long long get_time_ms(void)
{
    struct timeval tv;
    
    gettimeofday(&tv, NULL);
    return ((long long)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

long long get_elapsed_ms(t_simulation *sim)
{
    return (get_time_ms() - sim->start_time);
}

int parse_int_positive(char *str)
{
    int i;
    int result;
    
    if (!str || !str[0])
        return (-1);
    i = 0;
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (-1);
        i++;
    }
    result = 0;
    i = 0;
    while (str[i])
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result);
}

int parse_scheduler(char *str)
{
    if (!str)
        return (-1);
    if (strcmp(str, "fifo") == 0)
        return (FIFO);
    if (strcmp(str, "edf") == 0)
        return (EDF);
    return (-1);
}
