/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "mm.h"
#include "list.h"
#include "queue.h"
#include "stack.h"

void expr_bfs(void* expr_root, void (*func)(void* data))
{
    struct queue* queue = queue_create();
    queue_enqueue(queue, expr_root);

    while (queue->length != 0)
    {
        void* node = queue_dequeue(queue);
        func(node);

        if (expr_is_variadic_node(node))
        {
            struct variadic_node* var_node = (struct variadic_node*)node;
            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                queue_enqueue(queue, child);
                child = ((struct expr_node*)child)->next;
            }
        }
    }

    queue_free(queue, 0);
}

void expr_dfs(void* expr_root, void (*func)(void* node))
{
    struct stack* stack = stack_create();
    stack_push(stack, expr_root);

    while (stack->length != 0)
    {
        void* node = stack_pop(stack);
        if (expr_is_variadic_node(node))
        {
            struct variadic_node* var_node = (struct variadic_node*)node;
            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                stack_push(stack, child);
                child = ((struct expr_node*)child)->next;
            }
        }
        func(node);
    }

    stack_free(stack, 0);
}

void expr_top_down_dfs(void* expr_root, void* data, void (*func)(void* dfs_node, void* data))
{
    struct stack* stack = stack_create();
    struct top_down_dfs_node* dfs_root = x_malloc(sizeof(struct top_down_dfs_node));
    dfs_root->node = expr_root;
    dfs_root->parent = NULL;
    dfs_root->depth = 0;
    stack_push(stack, dfs_root);

    while (stack->length != 0)
    {
        void* dfs_node = stack_pop(stack);
        void* node = ((struct top_down_dfs_node*)dfs_node)->node;

        if (expr_is_variadic_node(node))
        {
            struct variadic_node* var_node = (struct variadic_node*)node;

            struct stack* reversed_children = stack_create();
            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                stack_push(reversed_children, child);
                child = ((struct expr_node*)child)->next;
            }

            while(reversed_children->length != 0)
            {
                struct top_down_dfs_node* dfs_child = x_malloc(sizeof(struct top_down_dfs_node));
                dfs_child->node = stack_pop(reversed_children);
                dfs_child->parent = node;
                dfs_child->depth = ((struct top_down_dfs_node*)dfs_node)->depth + 1;

                stack_push(stack, dfs_child);
            }

            stack_free(reversed_children, 0);
        }

        func(dfs_node, data);
        x_free(dfs_node);
    }

    stack_free(stack, 0);
}

static u64 expr_children_count(void* node)
{
    if (expr_is_variadic_node(node))
    {
        struct variadic_node* var_node = (struct variadic_node*)node;
        return var_node->length;
    }
    return 0;
}

void expr_bottom_up_dfs(void* expr_root, void* data, void (*func)(void* dfs_node, void* data))
{
    struct stack* stack = stack_create();
    struct bottom_up_dfs_node* dfs_root = x_malloc(sizeof(struct bottom_up_dfs_node));
    dfs_root->node = expr_root;
    dfs_root->parent = NULL;
    dfs_root->dfs_node_parent = NULL;
    dfs_root->depth = 0;
    dfs_root->children_count = expr_children_count(expr_root);
    dfs_root->children_processed = 0;
    stack_push(stack, dfs_root);

    while (stack->length != 0)
    {
        struct bottom_up_dfs_node* dfs_node = stack_peek(stack);
        if (dfs_node->children_count == dfs_node->children_processed)
        {
           stack_pop(stack);

           func(dfs_node, data);

           if (dfs_node->dfs_node_parent != NULL)
           {
               dfs_node->dfs_node_parent->children_processed++;
           }

            x_free(dfs_node);
            continue;
        }

        void* node = ((struct bottom_up_dfs_node*)dfs_node)->node;

        if (expr_is_variadic_node(node))
        {
            struct variadic_node* var_node = (struct variadic_node*)node;

            struct stack* reversed_children = stack_create();
            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                stack_push(reversed_children, child);
                child = ((struct expr_node*)child)->next;
            }

            while(reversed_children->length != 0)
            {
                struct bottom_up_dfs_node* dfs_child = x_malloc(sizeof(struct bottom_up_dfs_node));
                dfs_child->node = stack_pop(reversed_children);
                dfs_child->parent = node;
                dfs_child->dfs_node_parent = dfs_node;
                dfs_child->depth = dfs_node->depth + 1;
                dfs_child->children_count = expr_children_count(dfs_child->node);
                dfs_child->children_processed = 0;

                stack_push(stack, dfs_child);
            }

            stack_free(reversed_children, 0);
        }
    }

    stack_free(stack, 0);
}
