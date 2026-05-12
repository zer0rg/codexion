#include "codexion.h"

int main(int argc, char **argv)
{
    t_simulation sim;
    
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
