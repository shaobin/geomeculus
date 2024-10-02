/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"

/*
 * Clifford algebra to geometric calculus, p18
 */
void* expr_project(void* b, void* A)
{
    struct list* exprs = list_create();
    list_append(exprs, b);
    list_append(exprs, A);

    void* b_dot_A = expr_inner_product(exprs);
    list_free(exprs, 0);

    void* A_inverse = expr_inverse_blade(A);

    void* b_dot_A__A_inverse = expr_geometric_product(b_dot_A, A_inverse);
    expr_free_tree(b_dot_A);
    expr_free_tree(A_inverse);

    return b_dot_A__A_inverse;
}

void* func_project(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* b = child;

        child = child->next;
        void* A = child;
        return expr_project(b, A);
    }
    return expr_clone_tree(func);
}

/*
 * Clifford algebra to geometric calculus, p18
 */
void* expr_reject(void* b, void* A)
{
    struct list* exprs = list_create();
    list_append(exprs, b);
    list_append(exprs, A);
    void* b_wedge_A = expr_outer_product(exprs);
    list_free(exprs, 0);

    void* A_inverse = expr_inverse_blade(A);

    void* b_wedge_A__A_inverse = expr_geometric_product(b_wedge_A, A_inverse);
    expr_free_tree(b_wedge_A);
    expr_free_tree(A_inverse);

    return b_wedge_A__A_inverse;
}

void* func_reject(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* b = child;

        child = child->next;
        void* A = child;
        return expr_reject(b, A);
    }
    return expr_clone_tree(func);
}
