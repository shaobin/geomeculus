/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

void* expr_expand_reduce(void* expr)
{
    void* expanded_expr = expr_expand(expr);
    void* reduced_expr = expr_reduce(expanded_expr);
    expr_free_tree(expanded_expr);
    return reduced_expr;
}

void* func_expand_reduce(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_expand_reduce(var_node->head);
    }

    return expr_clone_tree(func);
}
