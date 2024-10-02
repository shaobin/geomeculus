/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <math.h>
#include "expr.h"

/*
 * Clifford algebra to geometric calculus, p14
 * Not all multivectors have inverses.
 * Blades are invertible if they have a nonzero norm.
 */
void* expr_inverse_blade(void* A)
{
    void* A_reverse = expr_reverse(A);
    void* scalar_prod = expr_scalar_product(A_reverse, A);

    void* coeff = expr_create_function_node("reciprocal");
    expr_variadic_node_append_child(coeff, scalar_prod);

    void* inverse = expr_create_geometric_product_node();
    expr_variadic_node_append_child(inverse, coeff);
    expr_variadic_node_append_child(inverse, A_reverse);

    void* simplified_inverse = expr_expand_reduce(inverse);
    expr_free_tree(inverse);
    return simplified_inverse;
}

void* func_inverse_blade(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct expr_node* child = var_node->head;
        return expr_inverse_blade(child);
    }
    return NULL;
}
