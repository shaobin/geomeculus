/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

/*
 * Clifford algebra to geometric calculus, p5
 */
void* expr_reverse(void* expr)
{
    // reverse(a) = a
    if (expr_is_leaf_node(expr))
    {
        return expr_clone_tree(expr);
    }

    if (expr_is_scalar_function_node(expr))
    {
        return expr_clone_tree(expr);
    }

    // reverse(A B) = reverse(B) reverse(A)
    if (expr_is_geometric_product_node(expr))
    {
        struct variadic_node* var_node = expr;
        struct variadic_node* reversed_node = expr_create_geometric_product_node();

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            void* reversed_child = expr_reverse(child);
            expr_variadic_node_prepend_child(reversed_node, reversed_child);
            child = ((struct expr_node*)child)->next;
        }

        return reversed_node;
    }

    // reverse(A + B) = reverse(A) + reverse(B)
    if (expr_is_geometric_sum_node(expr))
    {
        struct variadic_node* var_node = expr;
        struct variadic_node* reversed_node = expr_create_geometric_sum_node();

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            void* reversed_child = expr_reverse(child);
            expr_variadic_node_append_child(reversed_node, reversed_child);
            child = ((struct expr_node*)child)->next;
        }

        return reversed_node;
    }

    return NULL;
}

void* func_reverse(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct expr_node* child = var_node->head;
        return expr_reverse(child);
    }
    return NULL;
}
