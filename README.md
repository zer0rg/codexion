*Este proyecto ha sido creado como parte del currículo de 42.*

# Codexion

## Descripción

Codexion es una simulación de sincronización de hilos que modela un grupo de programadores compitiendo por recursos compartidos (dongles USB) en un entorno de co-working. El proyecto implementa soluciones a problemas clásicos de concurrencia utilizando mutexes y variables de condición de POSIX threads.

El objetivo es que los programadores compilen código regularmente sin agotarse (burnout), gestionando la adquisición y liberación de dongles con enfriamiento, mientras un monitor detecta posibles agotamientos con precisión de menos de 10ms.

## Instrucciones

### Compilación

```bash
cd coders
make
```

### Ejecución

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

**Parámetros:**
- `number_of_coders`: Número de programadores (y dongles)
- `time_to_burnout`: Tiempo máximo sin compilar antes de agotarse (ms)
- `time_to_compile`: Duración de la compilación (ms)
- `time_to_debug`: Duración de la depuración (ms)
- `time_to_refactor`: Duración del refactoring (ms)
- `number_of_compiles_required`: Compilaciones mínimas por programador
- `dongle_cooldown`: Tiempo de enfriamiento del dongle tras liberarse (ms)
- `scheduler`: Política de arbitraje (`fifo` o `edf`)

**Ejemplo:**
```bash
./codexion 4 800 200 200 200 3 100 fifo
```

## Blocking Cases Handled

### 1. Prevención de Deadlock
Se implementa **resource ordering** para prevenir interbloqueos. Cada programador adquiere siempre primero el dongle con menor ID, eliminando la posibilidad de ciclos de espera circular (condición de Coffman de "wait-for graph circular").

### 2. Prevención de Inanición (Starvation)
Se utiliza una **cola de prioridad (heap)** para cada dongle. Con el scheduler EDF, el programador con el deadline más próximo tiene prioridad, garantizando que nadie sea postergado indefinidamente.

### 3. Gestión del Cooldown
Tras liberar un dongle, este permanece no disponible durante `dongle_cooldown` milisegundos. Los programadores en espera se bloquean con `pthread_cond_wait` hasta que el dongle esté disponible y hayan pasado el cooldown.

### 4. Detección Precisa de Burnout
El hilo monitor realiza polling cada 500 microsegundos, verificando si algún programador ha excedido su deadline. El log de burnout se imprime dentro de los 10ms requeridos.

### 5. Serialización del Log
Un mutex dedicado (`log_mutex`) garantiza que los mensajes de estado no se mezclen. Cada línea de log es atómica.

### 6. Caso Especial: 1 Programador
Con un solo programador, solo existe un dongle. El código maneja este caso tomando el mismo dongle dos veces (simulando izquierda y derecha) e imprimiendo dos mensajes de "has taken a dongle".

## Thread Synchronization Mechanisms

### pthread_mutex_t

Se utilizan tres tipos de mutex:

1. **Dongle mutex**: Protege el estado de cada dongle (disponible/en uso, cooldown_end)
2. **State mutex**: Protege el estado global de la simulación (stop_simulation) y de cada programador (state, deadline, compile_count)
3. **Log mutex**: Serializa la salida estándar

```c
// Ejemplo de uso en dongle_acquire
pthread_mutex_lock(&dongle->mutex);
if (!dongle->available || current_time < dongle->cooldown_end)
{
    heap_insert(dongle->wait_queue, coder_id, priority, get_time_ms());
    pthread_cond_wait(&dongle->cond, &dongle->mutex);
}
dongle->available = 0;
pthread_mutex_unlock(&dongle->mutex);
```

### pthread_cond_t

Cada dongle tiene una variable de condición asociada:

1. **Espera de disponibilidad**: Los programadores esperan con `pthread_cond_wait` cuando el dongle no está disponible
2. **Broadcast de liberación**: Al liberar un dongle, se usa `pthread_cond_broadcast` para despertar a todos los que esperan

```c
// En dongle_release
dongle->available = 1;
dongle->cooldown_end = get_time_ms() + dongle->sim->dongle_cooldown;
pthread_cond_broadcast(&dongle->cond);
```

### Implementación del Scheduler

La cola de prioridad se implementa como un heap binario:

```c
typedef struct s_priority_queue {
    t_heap_node *nodes;
    int         size;
    int         capacity;
} t_priority_queue;
```

- **FIFO**: La prioridad es el timestamp de la solicitud
- **EDF**: La prioridad es el deadline (`last_compile_start + time_to_burnout`)

### Comunicación Thread-Safe

1. **Inicio sincronizado**: Todos los hilos esperan en `start_cond` hasta que todos estén listos
2. **Detección de stop**: Cada programador verifica `stop_simulation` antes de cada operación
3. **Liberación segura**: Si la simulación se detiene durante la adquisición de dongles, se liberan y se termina

```c
// Ejemplo en coder_routine
if (should_stop(coder->sim))
{
    dongle_release(first);
    return (0);
}
```

## Recursos

### Documentación de referencia
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [The Little Book of Semaphores](https://greenteapress.com/wp/semaphores/)
- [Deadlock Prevention](https://en.wikipedia.org/wiki/Deadlock_prevention_algorithms)

### Uso de IA
Se utilizó IA para:
- Diseño inicial de la estructura de datos
- Revisión del algoritmo de prevención de deadlock
- Optimización del polling del monitor para precisión <10ms
