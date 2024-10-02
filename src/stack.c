/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "stack.h"
#include "mm.h"

struct stack* stack_create()
{
    struct stack* stack = x_malloc(sizeof(struct stack));
    stack->length = 0;
    stack->head = NULL;
    return stack;
}

void stack_push(struct stack* stack, void* data)
{
    struct stack_node* node = x_malloc(sizeof(struct stack_node));
    node->data = data;
    node->next = stack->head;
    stack->head = node;
    stack->length++;
}

void* stack_pop(struct stack* stack)
{
    if (stack->length == 0)
    {
        return NULL;
    }
    struct stack_node* node = stack->head;
    stack->head = node->next;
    stack->length--;
    void* data = node->data;
    x_free(node);
    return data;
}

void* stack_peek(struct stack* stack)
{
    if (stack->length == 0)
    {
        return NULL;
    }
    return stack->head->data;
}

void stack_iterate(struct stack* stack, void (*callback)(void* data))
{
    struct stack_node* node = stack->head;
    while (node != NULL)
    {
        callback(node->data);
        node = node->next;
    }
}

void stack_clear(struct stack* stack, u64 free_payload)
{
    while (stack->length > 0)
    {
        void* data = stack_pop(stack);
        if (free_payload)
        {
            x_free(data);
        }
    }
}

void stack_free(struct stack* stack, u64 free_payload)
{
    stack_clear(stack, free_payload);
    x_free(stack);
}

#ifdef MODULE_TEST
#include <stdio.h>
int main(void)
{
    mm_initialize();

    struct stack* stack = stack_create();
    for (u64 i = 0; i < 10; ++i)
    {
        stack_push(stack, (void*)i);
    }
    for (u64 i = 0; i < 10; ++i)
    {
        printf("%ld\n", (u64)stack_pop(stack));
    }
    stack_free(stack, 0);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
