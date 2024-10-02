/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "queue.h"
#include "mm.h"

struct queue* queue_create()
{
    struct queue* queue = x_malloc(sizeof(struct queue));
    queue->length = 0;
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void queue_enqueue(struct queue* queue, void* data)
{
    struct queue_node* node = x_malloc(sizeof(struct queue_node));
    node->data = data;
    node->next = NULL;
    if (queue->length == 0)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        queue->tail->next = node;
        queue->tail = node;
    }
    queue->length++;
}

void* queue_dequeue(struct queue* queue)
{
    if (queue->length == 0)
    {
        return NULL;
    }
    struct queue_node* node = queue->head;
    queue->head = node->next;
    queue->length--;
    void* data = node->data;
    x_free(node);
    return data;
}

void* queue_peek(struct queue* queue)
{
    if (queue->length == 0)
    {
        return NULL;
    }
    return queue->head->data;
}

void queue_clear(struct queue* queue, u64 free_payload)
{
    while (queue->length > 0)
    {
        void* data = queue_dequeue(queue);
        if (free_payload)
        {
            x_free(data);
        }
    }
}

void queue_free(struct queue* queue, u64 free_payload)
{
    queue_clear(queue, free_payload);
    x_free(queue);
}

#ifdef MODULE_TEST
#include <stdio.h>
int main(void)
{
    mm_initialize();

    struct queue* queue = queue_create();
    for (u64 i = 0; i < 10; ++i)
    {
        queue_enqueue(queue, (void*)i);
    }
    for (u64 i = 0; i < 10; ++i)
    {
        printf("%ld\n", (u64)queue_dequeue(queue));
    }
    queue_free(queue, 0);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
