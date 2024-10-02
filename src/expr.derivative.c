/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "expr.h"
#include "func_repo.h"

void* expr_partial_derivative(void* expr, u64 var_idx)
{
    if (expr_is_geometric_sum_node(expr))
    {
        void* pd_expr = expr_create_geometric_sum_node();
        struct variadic_node* var_node = expr;
        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            void* pd_child = expr_partial_derivative(child, var_idx);
            expr_variadic_node_append_child(pd_expr, pd_child);
            child = ((struct expr_node*)child)->next;
        }
        return pd_expr;
    }
    else if (expr_is_geometric_product_node(expr))
    {
        void* pd_expr = expr_create_geometric_sum_node();
        struct variadic_node* var_node = expr;

        /*
         * a_1' * a_2 * ... * a_n +
         * a_1 * a_2' * ... * a_n +
         * ... +
         * a_1 * a_2 * ... * a_n'
         */

        u64 child_count = var_node->length;
        for (u64 i = 0; i < child_count; ++i)
        {
            void* product_term = expr_create_geometric_product_node();
            void* child = var_node->head;
            for (u64 j = 0; j < child_count; ++j)
            {
                if (i == j)
                {
                    void* pd_child = expr_partial_derivative(child, var_idx);
                    expr_variadic_node_append_child(product_term, pd_child);
                }
                else
                {
                    void* clone_child = expr_clone_tree(child);
                    expr_variadic_node_append_child(product_term, clone_child);
                }
                child = ((struct expr_node*)child)->next;
            }
            expr_variadic_node_append_child(pd_expr, product_term);
        }
        return pd_expr;
    }
    else if (expr_is_scalar_variable_node(expr))
    {
        struct leaf_node* sym_node = expr;
        if (sym_node->symbol->type == SYMBOL_TYPE_NUMBER_REAL)
        {
            if (sym_node->symbol->u64_value == var_idx)
            {
                return expr_create_scalar_node_with_f64(1.0);
            }
        }
    }
    else if (expr_is_function_node(expr))
    {
        // Chain rule
        void* pd_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(pd_expr, func_derivative(expr));
        expr_variadic_node_append_child(pd_expr, expr_partial_derivative(expr_func_argument(expr), var_idx));
        return pd_expr;
    }

    return expr_create_scalar_node_with_f64(0.0);
}

void* expr_nth_partial_derivative(void* expr, u64 var_idx, u64 n)
{
    if (n == 0)
    {
        return expr_clone_tree(expr);
    }
    else if (n == 1)
    {
        return expr_partial_derivative(expr, var_idx);
    }
    else
    {
        void* pd_expr = expr_partial_derivative(expr, var_idx);
        void* result = expr_nth_partial_derivative(pd_expr, var_idx, n - 1);
        expr_free_tree(pd_expr);
        return result;
    }
}

void* func_partial_derivative(void* func_node)
{
    struct variadic_node* var_node = (struct variadic_node*)func_node;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;

        u64 var_id = 0;
        child = child->next;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            var_id = (u64)f64_value;
        }
        return expr_partial_derivative(var_node->head, var_id);
    }
    return expr_clone_tree(func_node);
}

void* func_nth_partial_derivative(void* func_node)
{
    struct variadic_node* var_node = (struct variadic_node*)func_node;
    if (var_node->length == 3)
    {
        struct expr_node* child = var_node->head;

        u64 var_id = 0;
        child = child->next;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            var_id = (u64)f64_value;
        }

        u64 n = 0;
        child = child->next;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            n = (u64)f64_value;
        }
        return expr_nth_partial_derivative(var_node->head, var_id, n);
    }
    return expr_clone_tree(func_node);
}

/*
 * Let expr represents a function multivector-valued F defined on an open set U of R^n.
*/
void* expr_gradient(void* expr, u64 n)
{
    void* grad_expr = expr_create_geometric_sum_node();
    for (u64 i = 1; i <= n; ++i)
    {
        void* prod_term = expr_create_geometric_product_node();

        expr_variadic_node_append_child(prod_term, expr_create_basis_vector_node(i));
        void* pd_expr = expr_partial_derivative(expr, i);
        expr_variadic_node_append_child(prod_term, pd_expr);

        expr_variadic_node_append_child(grad_expr, prod_term);
    }
    return grad_expr;
}

void* func_gradient(void* func_node)
{
    struct variadic_node* var_node = (struct variadic_node*)func_node;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;

        u64 n = 0;
        child = child->next;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            n = (u64)f64_value;
        }
        return expr_gradient(var_node->head, n);
    }
    return expr_clone_tree(func_node);
}

/*
 * Let F be a function multivector-valued function defined on manifold M in R^n.
 * M is a parametrized m-dimensional manifold.
 */
void* expr_vector_derivative(void* F, void* M, u64 m)
{
    struct list* tangent_basis = list_create();

    for (u64 i = 1; i <= m; ++i)
    {
        void* pd_expr = expr_partial_derivative(M, i);
        void* pd_expr_ = expr_expand_reduce(pd_expr);
        expr_free_tree(pd_expr);
        list_append(tangent_basis, pd_expr_);
    }

    struct list* reciprocal_basis = expr_reciprocal_basis(tangent_basis);

    for (u64 i = 0; i < tangent_basis->length; ++i)
    {
        expr_free_tree(list_get(tangent_basis, i));
    }
    list_free(tangent_basis, 0);

    void* vec_d_expr = expr_create_geometric_sum_node();

    for (u64 i = 1; i <= m; ++i)
    {
        void* prod_term = expr_create_geometric_product_node();

        expr_variadic_node_append_child(prod_term, list_get(reciprocal_basis, i - 1));

        void* pd_expr = expr_partial_derivative(F, i);
        expr_variadic_node_append_child(prod_term, pd_expr);

        expr_variadic_node_append_child(vec_d_expr, prod_term);
    }

    list_free(reciprocal_basis, 0);

    return vec_d_expr;
}

void* func_vector_derivative(void* func_node)
{
    struct variadic_node* var_node = (struct variadic_node*)func_node;
    if (var_node->length == 3)
    {
        struct expr_node* child = var_node->head;
        void* F = child;

        child = child->next;
        void* M = child;

        child = child->next;
        u64 m = 0;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            m = (u64)f64_value;
        }

        return expr_vector_derivative(F, M, m);
    }

    return expr_clone_tree(func_node);
}
