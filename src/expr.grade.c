/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

/*
 * A simplified term is a geometric product of basis vectors and constants.
 */
u64 expr_grade_of_simplified_term(void* expr)
{
    if (expr_is_geometric_product_node(expr))
    {
        struct variadic_node* var_node = expr;
        u64 grade = 0;

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            if (expr_is_basis_vector_node(child))
            {
                ++grade;
            }
            child = ((struct expr_node*)child)->next;
        }

        return grade;
    }
    else if (expr_is_basis_vector_node(expr))
    {
        return 1;
    }

    return 0;
}

/*
 * Get the k-vector part of a simplified expr.
 * A simplified expr is a geometric sum of simplified terms or a simplified term.
 */
void* expr_k_vector_of_simplified_expr(void* expr, u64 k)
{
    if (expr_is_geometric_sum_node(expr))
    {
        struct variadic_node* var_node = expr;
        struct variadic_node* k_vector = expr_create_geometric_sum_node();

        struct expr_node* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            u64 grade = expr_grade_of_simplified_term(child);
            if (grade == k)
            {
                expr_variadic_node_append_child(k_vector, expr_clone_tree(child));
            }
            child = child->next;
        }

        void* simp_k_vector = expr_remove_redundant_nodes(k_vector);
        expr_free_tree(k_vector);

        return simp_k_vector;
    }
    else if (expr_is_geometric_product_node(expr))
    {
        u64 grade = expr_grade_of_simplified_term(expr);
        if (grade == k)
        {
            return expr_clone_tree(expr);
        }
    }
    else if (expr_is_basis_vector_node(expr))
    {
        if (k == 1)
        {
            return expr_clone_tree(expr);
        }
    }
    else if (expr_is_scalar_node(expr))
    {
        if (k == 0)
        {
            return expr_clone_tree(expr);
        }
    }

    if (k == 0)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    return NULL;
}

void* expr_grade(void* expr, u64 k)
{
    void* simplified_expr = expr_expand_reduce(expr);
    void* k_vector = expr_k_vector_of_simplified_expr(simplified_expr, k);
    expr_free_tree(simplified_expr);
    return k_vector;
}

void* func_grade(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* expr = child;

        child = child->next;
        u64 k = 0;
        if (expr_is_instantiated_scalar_node(child))
        {
            f64 f64_value = ((struct leaf_node*)child)->f64_value;
            k = (u64)f64_value;
        }

        return expr_grade(expr, k);
    }

    return expr_clone_tree(func);
}
