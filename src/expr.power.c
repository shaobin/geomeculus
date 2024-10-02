/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

void* expr_power(void* expr, u64 n)
{
    if (n == 0)
    {
        return expr_create_scalar_node_with_f64(1.0);
    }
    else if (n == 1)
    {
        return expr_clone_tree(expr);
    }
    else
    {
        void* result = expr_create_geometric_product_node();

        for (u64 i = 0; i < n; ++i)
        {
            expr_variadic_node_append_child(result, expr_clone_tree(expr));
        }

        void* result_simplified = expr_expand_reduce(result);
        expr_free_tree(result);

        return result_simplified;
    }
}

void* func_power(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        child = child->next;

        u64 n = 0;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            n = (u64)f64_value;
        }
        return expr_power(var_node->head, n);
    }
    return expr_clone_tree(func);
}
