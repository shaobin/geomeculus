/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

void expr_variadic_node_prepend_child(void* node, void* child)
{
    struct variadic_node* var_node = (struct variadic_node*)node;
    struct expr_node* expr_child = child;

    expr_child->next = var_node->head;
    expr_child->prev = NULL;

    if (var_node->length == 0)
    {
        var_node->tail = child;
    }
    else
    {
        ((struct expr_node*)var_node->head)->prev = child;
    }

    var_node->head = child;
    ++(var_node->length);
}

void expr_variadic_node_append_child(void* node, void* child)
{
    struct variadic_node* var_node = (struct variadic_node*)node;
    struct expr_node* expr_child = child;

    expr_child->next = NULL;
    expr_child->prev = var_node->tail;

    if (var_node->length == 0)
    {
        var_node->head = child;
    }
    else
    {
        ((struct expr_node*)var_node->tail)->next = child;
    }

    var_node->tail = child;
    ++(var_node->length);
}

void expr_variadic_node_replace_child(void* node, void* child, void* new_child)
{
    struct variadic_node* var_node = node;
    struct expr_node* expr_child = child;
    struct expr_node* expr_new_child = new_child;

    struct expr_node* current_child = var_node->head;
    if (current_child == child)
    {
        expr_new_child->next = expr_child->next;
        expr_new_child->prev = NULL;

        var_node->head = new_child;

        if (var_node->length == 1)
        {
            var_node->tail = new_child;
        }
        else
        {
            ((struct expr_node*)expr_child->next)->prev = new_child;
        }

        expr_free_tree(child);
        return;
    }

    for (u64 i = 0; i < var_node->length - 1; ++i)
    {
        if (current_child->next == child)
        {
            expr_new_child->next = expr_child->next;
            expr_new_child->prev = current_child;
            current_child->next = new_child;

            if (expr_child->next == NULL)
            {
                var_node->tail = new_child;
            }
            else
            {
                ((struct expr_node*)expr_child->next)->prev = new_child;
            }

            expr_free_tree(child);
            return;
        }
        current_child = current_child->next;
    }
}

void expr_variadic_node_remove_child(void* node, void* child)
{
    struct variadic_node* var_node = (struct variadic_node*)node;
    struct expr_node* expr_child = (struct expr_node*)child;
    struct expr_node* current_child = var_node->head;

    if (current_child == child)
    {
        var_node->head = current_child->next;

        if (expr_child->next == NULL)
        {
            var_node->tail = NULL;
        }
        else
        {
            ((struct expr_node*)expr_child->next)->prev = NULL;
        }

        --(var_node->length);
        expr_free_tree(child);
        return;
    }

    for (u64 i = 0; i < var_node->length - 1; ++i)
    {
        if (((struct expr_node*)current_child)->next == child)
        {
            ((struct expr_node*)current_child)->next = ((struct expr_node*)child)->next;

            if (expr_child->next == NULL)
            {
                var_node->tail = current_child;
            }
            else
            {
                ((struct expr_node*)expr_child->next)->prev = current_child;
            }

            --(var_node->length);
            expr_free_tree(child);
            return;
        }
        current_child = ((struct expr_node*)current_child)->next;
    }
}
