#ifndef _MIN_HEAP_H
#define _MIN_HEAP_H

using min_heap_t = struct min_heap_st*;
using heap_node_t = struct heap_node_st*;

struct heap_node_st {
    int prio; /* 优先级 */
    int index; /* node在heap中的数组下标 */
    void *data; /* 用户userdata, 无需关心 */
};

struct min_heap_st {
    int heap_size; /* heap数组的尺寸 */
    int heap_length; /* heap数组存储的node个数 */
    heap_node_t *heap_nodes; /* node指针数组, realloc按需扩容 */
};

min_heap_t create_heap();
void destroy_heap(min_heap_t heap);
int heap_size(min_heap_t heap);
int insert_node(min_heap_t heap, heap_node_t node);
int remove_node(min_heap_t heap, heap_node_t node);
int modify_node(min_heap_t heap, heap_node_t node);
heap_node_t top_node(min_heap_t heap);
heap_node_t pop_node(min_heap_t heap);

#endif
