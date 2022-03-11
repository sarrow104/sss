#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gtest/gtest.h>

#include "../heap/min_heap.h"

int test01()
{
    heap_node_t nodes = static_cast<heap_node_st*>(calloc(1024, sizeof(*nodes)));

    srand((unsigned int)time(nullptr));

    min_heap_t heap = create_heap();

    int i;
    for (i = 0; i < 1024; ++i) {
        nodes[i].prio = rand();
        nodes[i].data = malloc(sizeof(int));
        *(int *)nodes[i].data = i;

        printf("insert_node=%d\n", insert_node(heap, nodes + i));
    }

    for (i = 0; i < 1024; ++i) {
        heap_node_t node = pop_node(heap);
        if (node) {
            printf("[pop_node] prio=%d data=%d\n", node->prio, *(int *)node->data);
            free(node->data);
        }
    }

    free(nodes);
    destroy_heap(heap);
    return 0;
}

int test02()
{
    time_t s = time(nullptr);

    min_heap_t heap = create_heap();

    srand(static_cast<unsigned int>(time(nullptr)));

    int i, add = 0, del = 0;
    // const auto loop_cnt = 10000000;
    const auto loop_cnt = 100000;
    for (i = 0; i < loop_cnt; ++i) { 
        heap_node_t node = static_cast<heap_node_t>(calloc(1, sizeof(*node)));
        node->prio = rand();
        //printf("%d\n", node->prio);
        if (!insert_node(heap, node)) {
            ++add;
        }
    }

    printf("time02_add_used=%d\n", time(NULL) - s);

    s = time(NULL);

    while (heap_size(heap)) {
        heap_node_t node = pop_node(heap);
        //printf("prio=%d\n", node->prio);
        free(node);
        ++del;
    }

    destroy_heap(heap);

    time_t e = time(NULL);

    printf("test02_del_used=%d add=%d del=%d\n", e - s, add, del);

    return 0;
}

void test03()
{
    min_heap_t heap = create_heap();

    int add = 0;
    int del = 0;

    heap_node_t node58 = nullptr;
    heap_node_t node102 = nullptr;

    int i;
    for (i = 1023; i >= 0; --i) {
        heap_node_t node = static_cast<heap_node_t>(calloc(1, sizeof(*node)));
        node->prio = i;
        if (!insert_node(heap, node))
            ++add;
        if (node->prio == 58)
            node58 = node;
        if (node->prio == 102)
            node102 = node;
    }

    if (remove_node(heap, node58) == -1) {
        fprintf(stderr, "remove node 58 fail\n");
        free(node58);
    }
    if (remove_node(heap, node102) == -1) {
        fprintf(stderr, "remove node 102 fail\n");
        free(node102);
    }

    while (heap_size(heap)) {
        heap_node_t node = pop_node(heap);
        printf("prio=%d\n", node->prio);
        free(node);
    }

    destroy_heap(heap);
}

void test04()
{
    min_heap_t heap = create_heap();

    int add = 0;
    int del = 0;

    heap_node_t node58 = nullptr;
    heap_node_t node102 = nullptr;

    int i = 0;
    for (i = 1023; i >= 0; --i) {
        heap_node_t node = static_cast<heap_node_t>(calloc(1, sizeof(*node)));
        node->prio = i;
        if (!insert_node(heap, node))
            ++add;
        if (node->prio == 58)
            node58 = node;
        if (node->prio == 102)
            node102 = node;
    }

    node58->prio = 77;
    if (modify_node(heap, node58) == -1) {
        fprintf(stderr, "modify node 58 -> 77 \n");
    }
    node102->prio = 88;
    if (modify_node(heap, node102) == -1) {
        fprintf(stderr, "modify node 102 -> 88 \n");
    }
    while (heap_size(heap)) {
        heap_node_t node = pop_node(heap);
        printf("prio=%d\n", node->prio);
        free(node);
    }

    destroy_heap(heap);
}

TEST(minheap, all)
{
    test01();
    test02(); /* why delete that slow ?*/
    test03();
    test04();
}

