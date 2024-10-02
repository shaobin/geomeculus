/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>

#include "expr.h"
#include "list.h"

/*
 * Ref: Clifford algebra to geometric calculus, p28
 */
struct list* expr_reciprocal_basis(struct list* basis)
{
    void* An = expr_outer_product(basis);
    void* An_reverse = expr_reverse(An);

    void* An_An_reverse = expr_create_geometric_product_node();
    expr_variadic_node_append_child(An_An_reverse, An);
    expr_variadic_node_append_child(An_An_reverse, expr_clone_tree(An_reverse)); // An_reverse will be used later

    struct leaf_node* An_An_reverse_ = expr_expand_reduce(An_An_reverse);
    expr_free_tree(An_An_reverse);

    struct list* reciprocal_basis = list_create();

    for (u64 k = 0; k < basis->length; ++k)
    {
        f64 coeff = 1.0;
        if (k % 2) // k is zero based, k is 1-based in eq 3.7
        {
            coeff = -1.0;
        }

        struct list* exprs_An_x_k = list_create();
        for (u64 i = 0; i < basis->length; ++i)
        {
            if (i == k)
            {
                continue;
            }
            void* expr = list_get(basis, i);
            list_append(exprs_An_x_k, expr);
        }

        void *An_x_k = expr_outer_product(exprs_An_x_k);
        list_free(exprs_An_x_k, 0);

        void *a_k = expr_create_geometric_product_node();
        expr_variadic_node_append_child(a_k, expr_create_scalar_node_with_f64(coeff));

        struct variadic_node* denominator = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(denominator, expr_clone_tree(An_An_reverse_));

        expr_variadic_node_append_child(a_k, denominator);
        expr_variadic_node_append_child(a_k, An_x_k);
        expr_variadic_node_append_child(a_k, expr_clone_tree(An_reverse));

        void* simplified_a_k = expr_expand_reduce(a_k);
        expr_free_tree(a_k);
        list_append(reciprocal_basis, simplified_a_k);
    }

    expr_free_tree(An_An_reverse_);
    expr_free_tree(An_reverse);

    return reciprocal_basis;
}

void expr_print_basis(struct list* basis)
{
    printf("basis:\n");
    for (u64 i = 0; i < basis->length; ++i)
    {
        void* vec = list_get(basis, i);
        expr_print_infix(vec);
    }
    printf("\n");
}
