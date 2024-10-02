/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include "expr.h"
#include "config.h"
#include "special_func.h"

/*
 * Ref: Geometric algebra for computer science, section 21.3, p531
 */
static void* expr_exp_series(void* A, u64 n_terms)
{
    u64 n = n_terms > 2 ? n_terms : 2;

    void* expr_mag = expr_magnitude(A);
    u64 mag = 1;
    if (expr_is_instantiated_scalar_node(expr_mag))
    {
        f64 mag_value = ((struct leaf_node*)expr_mag)->f64_value;
        mag = 2 << llround(ceil(log2(mag_value)));
    }
    expr_free_tree(expr_mag);

    void* expr = NULL;

    if (mag > 1)
    {
        // when mag > 1, we need to scale down the input to avoid overflow
        void* B = expr_create_geometric_product_node();
        expr_variadic_node_append_child(B, expr_create_scalar_node_with_f64(1.0 / mag));
        expr_variadic_node_append_child(B, expr_clone_tree(A));

        expr = expr_expand_reduce(B);
        expr_free_tree(B);
    }
    else
    {
        expr = A;
    }

    void* result = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(result, expr_create_scalar_node_with_f64(1.0));
    expr_variadic_node_append_child(result, expr_clone_tree(expr));

    void* expr_powers[n];
    expr_powers[1] = expr;

    for (u64 i = 2; i < n; ++i)
    {
        expr_powers[i] = expr_geometric_product(expr_powers[i - 1], expr);

        if (expr_is_instantiated_scalar_node(expr_powers[i]))
        {
            struct leaf_node* leaf = expr_powers[i];
            if (leaf->f64_value == 0.0)
            {
                expr_free_tree(expr_powers[i]);
                break;
            }
        }

        void* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(factorial_reciprocal(i)));
        expr_variadic_node_append_child(prod, expr_powers[i]);

        expr_variadic_node_append_child(result, prod);
    }

    void* result_simplified = expr_expand_reduce(result);
    expr_free_tree(result);

    if (mag > 1)
    {
        i64 square_count = llround(log2(mag)) - 1;
        void* sq = expr_geometric_product(result_simplified, result_simplified);

        for (i64 i = 0; i < square_count; ++i)
        {
            void* sq_sq = expr_geometric_product(sq, sq);
            expr_free_tree(sq);
            sq = sq_sq;
        }

        expr_free_tree(expr); // in this case expr is not the original input A, so we need to free it
        expr_free_tree(result_simplified);
        return sq;
    }

    return result_simplified;
}

void* expr_exponential(void* expr)
{
    return expr_exp_series(expr, exponential_series_expansion_order);
}

void* func_exponential(void* func)
{
    struct variadic_node* var_node = func;
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
        return expr_exp_series(var_node->head, n);
    }
    else if (var_node->length == 1)
    {
        return expr_exp_series(var_node->head, exponential_series_expansion_order);
    }

    return expr_clone_tree(func);
}
