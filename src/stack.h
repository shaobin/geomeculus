/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef STACK_H
#define STACK_H

#include "numeric_types.h"

struct stack_node
{
    void* data;
    struct stack_node* next;  // along the "head -> tail" direction
};

struct stack
{
    u64 length;
    struct stack_node* head;
};

struct stack* stack_create();
void stack_push(struct stack* stack, void* data);
void* stack_pop(struct stack* stack);
void* stack_peek(struct stack* stack);
void stack_iterate(struct stack* stack, void (*callback)(void* data));
void stack_clear(struct stack* stack, u64 free_payload);
void stack_free(struct stack* stack, u64 free_payload);

#endif
