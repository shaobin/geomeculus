/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

u64 expr_node_cardinality(void* node)
{
    if (expr_is_leaf_node(node))
    {
        return 1;
    }

    struct variadic_node* var_node = (struct variadic_node*)node;
    u64 cardinality = 0;
    void* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        cardinality += expr_node_cardinality(child);
        child = ((struct expr_node*)child)->next;
    }
    return cardinality;
}

// count the basic arithmetic operations in the expression tree
u64 expr_node_arithmetic_op_count(void* node)
{
    if (expr_is_leaf_node(node))
    {
        return 0;
    }

    struct variadic_node* var_node = (struct variadic_node*)node;
    u64 op_count = var_node->length - 1;
    void* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        op_count += expr_node_arithmetic_op_count(child);
        child = ((struct expr_node*)child)->next;
    }

    return op_count;
}
