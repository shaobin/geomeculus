/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef QUEUE_H
#define QUEUE_H

#include "numeric_types.h"

struct queue_node
{
    void* data;
    struct queue_node* next;  // along the "head -> tail" direction
};

struct queue
{
    u64 length;
    struct queue_node* head;
    struct queue_node* tail;
};

struct queue* queue_create();
void queue_enqueue(struct queue* queue, void* data);
void* queue_dequeue(struct queue* queue);
void* queue_peek(struct queue* queue);
void queue_clear(struct queue* queue, u64 free_payload);
void queue_free(struct queue* queue, u64 free_payload);

#endif
