/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

void* expr_magnitude(void* expr)
{
    void* reversed_expr = expr_reverse(expr);

    void* result = expr_create_function_node("sqrt");
    void* scalar_prod = expr_scalar_product(reversed_expr, expr);
    expr_variadic_node_append_child(result, scalar_prod);

    expr_free_tree(reversed_expr);

    void* simplified = expr_evaluate_function(result);
    expr_free_tree(result);

    return simplified;
}

void* func_magnitude(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_magnitude(var_node->head);
    }
    return expr_clone_tree(func);
}

void* expr_magnitude_squared(void* expr)
{
    void* reversed_expr = expr_reverse(expr);
    void* scalar_prod = expr_scalar_product(reversed_expr, expr);
    expr_free_tree(reversed_expr);

    void* simplified = expr_evaluate_function(scalar_prod);
    expr_free_tree(scalar_prod);

    return simplified;
}

void* func_magnitude_squared(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_magnitude_squared(var_node->head);
    }
    return expr_clone_tree(func);
}
