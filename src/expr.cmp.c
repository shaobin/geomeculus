/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include "expr.h"

u64 expr_compare(void* A, void* B)
{
    if (A == B)
    {
        return 1;
    }

    if (A == NULL || B == NULL)
    {
        return 0;
    }

    if (((struct expr_node*)A)->type != ((struct expr_node*)B)->type)
    {
        return 0;
    }

    if (expr_is_variadic_node(A))
    {
        struct variadic_node* var_node_A = (struct variadic_node*)A;
        struct variadic_node* var_node_B = (struct variadic_node*)B;

        if (var_node_A->length != var_node_B->length)
        {
            return 0;
        }

        void* child_A = var_node_A->head;
        void* child_B = var_node_B->head;
        for (u64 i = 0; i < var_node_A->length; ++i)
        {
            if (!expr_compare(child_A, child_B))
            {
                return 0;
            }
            child_A = ((struct expr_node*)child_A)->next;
            child_B = ((struct expr_node*)child_B)->next;
        }

        return 1;
    }
    else if (expr_is_leaf_node(A))
    {
        struct leaf_node* leaf_node_A = (struct leaf_node*)A;
        struct leaf_node* leaf_node_B = (struct leaf_node*)B;

        if (expr_is_instantiated_scalar_node(A))
        {
            return f64_is_close(leaf_node_A->f64_value, leaf_node_B->f64_value);
        }

        return symbol_compare(leaf_node_A->symbol, leaf_node_B->symbol) && (leaf_node_A->u64_value == leaf_node_B->u64_value);
    }
    else
    {
        printf("unknown node type: %lu\n", ((struct expr_node*)A)->type);
        return 0;
    }
}
