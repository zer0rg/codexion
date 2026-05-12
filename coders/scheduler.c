#include "codexion.h"

static void heap_swap(t_heap_node *a, t_heap_node *b)
{
    t_heap_node temp;
    
    temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify_up(t_priority_queue *heap, int index)
{
    int parent;
    
    while (index > 0)
    {
        parent = (index - 1) / 2;
        if (heap->nodes[index].priority < heap->nodes[parent].priority)
        {
            heap_swap(&heap->nodes[index], &heap->nodes[parent]);
            index = parent;
        }
        else
            break;
    }
}

static void heapify_down(t_priority_queue *heap, int index)
{
    int left;
    int right;
    int smallest;
    
    while (1)
    {
        left = 2 * index + 1;
        right = 2 * index + 2;
        smallest = index;
        
        if (left < heap->size && heap->nodes[left].priority < heap->nodes[smallest].priority)
            smallest = left;
        if (right < heap->size && heap->nodes[right].priority < heap->nodes[smallest].priority)
            smallest = right;
        
        if (smallest != index)
        {
            heap_swap(&heap->nodes[index], &heap->nodes[smallest]);
            index = smallest;
        }
        else
            break;
    }
}

t_priority_queue *heap_create(int capacity)
{
    t_priority_queue *heap;
    
    heap = malloc(sizeof(t_priority_queue));
    if (!heap)
        return (NULL);
    heap->nodes = malloc(sizeof(t_heap_node) * capacity);
    if (!heap->nodes)
    {
        free(heap);
        return (NULL);
    }
    heap->size = 0;
    heap->capacity = capacity;
    return (heap);
}

void heap_destroy(t_priority_queue *heap)
{
    if (heap)
    {
        if (heap->nodes)
            free(heap->nodes);
        free(heap);
    }
}

void heap_insert(t_priority_queue *heap, int coder_id, long long priority, long long request_time)
{
    if (heap->size >= heap->capacity)
        return;
    
    heap->nodes[heap->size].coder_id = coder_id;
    heap->nodes[heap->size].priority = priority;
    heap->nodes[heap->size].request_time = request_time;
    heapify_up(heap, heap->size);
    heap->size++;
}

int heap_extract_min(t_priority_queue *heap, long long *priority)
{
    int coder_id;
    
    if (heap->size == 0)
        return (-1);
    
    coder_id = heap->nodes[0].coder_id;
    if (priority)
        *priority = heap->nodes[0].priority;
    
    heap->size--;
    if (heap->size > 0)
    {
        heap->nodes[0] = heap->nodes[heap->size];
        heapify_down(heap, 0);
    }
    
    return (coder_id);
}

int heap_peek(t_priority_queue *heap)
{
    if (heap->size == 0)
        return (-1);
    return (heap->nodes[0].coder_id);
}

int heap_is_empty(t_priority_queue *heap)
{
    return (heap->size == 0);
}

void heap_remove(t_priority_queue *heap, int coder_id)
{
    int i;
    
    i = 0;
    while (i < heap->size)
    {
        if (heap->nodes[i].coder_id == coder_id)
        {
            heap->size--;
            if (i < heap->size)
            {
                heap->nodes[i] = heap->nodes[heap->size];
                heapify_up(heap, i);
                heapify_down(heap, i);
            }
            return;
        }
        i++;
    }
}

void heap_update_priority(t_priority_queue *heap, int coder_id, long long new_priority)
{
    int i;
    long long old_priority;
    
    i = 0;
    while (i < heap->size)
    {
        if (heap->nodes[i].coder_id == coder_id)
        {
            old_priority = heap->nodes[i].priority;
            heap->nodes[i].priority = new_priority;
            if (new_priority < old_priority)
                heapify_up(heap, i);
            else
                heapify_down(heap, i);
            return;
        }
        i++;
    }
}
