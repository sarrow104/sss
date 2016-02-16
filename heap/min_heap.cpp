#include "min_heap.h"
#include <stdlib.h>
#include <assert.h>

#define PARENT(node) (node->index  >> 1)
#define LEFT(node)   (node->index  << 1)
#define RIGHT(node)  ((node->index << 1) + 1)

min_heap_t create_heap()
{
    min_heap_t heap = reinterpret_cast<min_heap_t>(calloc(1, sizeof(*heap)));
    return heap;
}

void destroy_heap(min_heap_t heap)
{
    if (!heap)
        return;
    if (heap->heap_nodes)
        free(heap->heap_nodes);
    free(heap);
}

static void upheap(min_heap_t heap, heap_node_t node)
{
    int parent = PARENT(node);

    while (parent) {
        if (node->prio < heap->heap_nodes[parent]->prio) {
            heap->heap_nodes[parent]->index = node->index;
            heap->heap_nodes[node->index] = heap->heap_nodes[parent];
            node->index = parent;
        } else
            break;
        parent = PARENT(node);
    }

    heap->heap_nodes[node->index] = node;
}

static void downheap(min_heap_t heap, heap_node_t node)
{
    for (;;) {
        int left  = LEFT(node);
        int right = RIGHT(node);
        int min_prio = node->prio;
        int min_index = node->index;

        if (left <= heap->heap_length &&
                heap->heap_nodes[left]->prio < min_prio) {
            min_index = left;
            min_prio = heap->heap_nodes[left]->prio;
        }
        if (right <= heap->heap_length &&
                heap->heap_nodes[right]->prio < min_prio) {
            min_index = right;
        }

        if (min_index != node->index) {
            heap->heap_nodes[min_index]->index = node->index;
            heap->heap_nodes[node->index] = heap->heap_nodes[min_index];
            node->index = min_index;
        } else
            break;
    }
    heap->heap_nodes[node->index] = node;
}

static void adjust_heap(min_heap_t heap, heap_node_t node)
{
    int parent = PARENT(node);

    if (parent && node->prio < heap->heap_nodes[parent]->prio)
        upheap(heap, node);
    else
        downheap(heap, node);
}

int insert_node(min_heap_t heap, heap_node_t node)
{
    if (!heap || !node || node->prio < 0)
        return -1;

    if (heap->heap_length >= heap->heap_size) {
        int new_size = heap->heap_size + 1;

        heap_node_t *new_nodes = (heap_node_t *)realloc(heap->heap_nodes,
                (new_size + 1) * sizeof(heap_node_t));
        if (!new_nodes)
            return -1;

        heap->heap_nodes = new_nodes;
        heap->heap_size++;
    }

    node->index = ++heap->heap_length;
    adjust_heap(heap, node);
    return 0;
}

int remove_node(min_heap_t heap, heap_node_t node)
{
    if (!heap || !node || !heap->heap_length || node->index <= 0 || node->index > heap->heap_length)
        return -1;

    assert(node == heap->heap_nodes[node->index]);

    if (node->index == heap->heap_length)
        heap->heap_length--;
    else {
        heap->heap_nodes[node->index] = heap->heap_nodes[heap->heap_length--];
        heap->heap_nodes[node->index]->index = node->index;
        adjust_heap(heap, heap->heap_nodes[node->index]);
    }
    return 0;
}

int modify_node(min_heap_t heap, heap_node_t node)
{
    if (!heap || !node || !heap->heap_length || node->index <= 0 ||
            node->index > heap->heap_length)
        return -1;

    assert(node == heap->heap_nodes[node->index]);

    adjust_heap(heap, node);

    return 0;
}

heap_node_t pop_node(min_heap_t heap)
{
    if (!heap || !heap->heap_length)
        return NULL;

    heap_node_t top = heap->heap_nodes[1];

    if (heap->heap_length == 1)
        heap->heap_length = 0;
    else {
        heap->heap_nodes[1] = heap->heap_nodes[heap->heap_length--];
        heap->heap_nodes[1]->index = 1;
        adjust_heap(heap, heap->heap_nodes[1]);
    }

    return top;
}

heap_node_t top_node(min_heap_t heap)
{
    if (!heap || !heap->heap_length)
        return NULL;
    return heap->heap_nodes[1];
}

int heap_size(min_heap_t heap)
{
    if (!heap)
        return -1;
    return heap->heap_length;
}
