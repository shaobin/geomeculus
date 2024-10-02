/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <math.h>
#include "expr.h"

/*
    Rotates v by an angle theta in a plane specified by bivector B
    exp(-B * theta / 2) * v * exp(B * theta / 2)
    exp(B * theta / 2) = cos(theta / 2) + B * sin(theta / 2)
    exp(-B * theta / 2) = cos(theta / 2) - B * sin(theta / 2)

    let a = sin(theta / 2) and b = cos(theta / 2)

    exp(-B * theta / 2) * v * exp(B * theta / 2)
    = (b - a * B) * v * (b + a * B)
    = b * b * v - a * b * B * v + a * b * v * B  - a * a * B * v * B

Python version:
def rotate(v, B, theta):
    a = math.sin(theta / 2)
    b = math.cos(theta / 2)
    return (
        GeometricAddNode(
            [
                GeometricProductNode([ConstantNode(b * b), v.clone()]),
                GeometricProductNode([ConstantNode(-a * b), B.clone(), v.clone()]),
                GeometricProductNode([ConstantNode(a * b), v.clone(), B.clone()]),
                GeometricProductNode(
                    [ConstantNode(-a * a), B.clone(), v.clone(), B.clone()]
                ),
            ]
        )
        .expand()
        .reduce()
    )
*/
void* expr_rotate_vector_in_plane(void* v, void* B, f64 theta)
{
    f64 a = sin(theta / 2);
    f64 b = cos(theta / 2);

    void* bb = expr_create_scalar_node_with_f64(b * b);
    void* _ab = expr_create_scalar_node_with_f64(-a * b);
    void* ab = expr_create_scalar_node_with_f64(a * b);
    void* _aa = expr_create_scalar_node_with_f64(-a * a);

    void* sum = expr_create_geometric_sum_node();

    void* _aaBvB = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(_aaBvB, expr_clone_tree(B));
    expr_variadic_node_prepend_child(_aaBvB, expr_clone_tree(v));
    expr_variadic_node_prepend_child(_aaBvB, expr_clone_tree(B));
    expr_variadic_node_prepend_child(_aaBvB, _aa);
    expr_variadic_node_prepend_child(sum, _aaBvB);

    void* abvB = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(abvB, expr_clone_tree(B));
    expr_variadic_node_prepend_child(abvB, expr_clone_tree(v));
    expr_variadic_node_prepend_child(abvB, ab);
    expr_variadic_node_prepend_child(sum, abvB);

    void* _abBv = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(_abBv, expr_clone_tree(v));
    expr_variadic_node_prepend_child(_abBv, expr_clone_tree(B));
    expr_variadic_node_prepend_child(_abBv, _ab);
    expr_variadic_node_prepend_child(sum, _abBv);

    void* bbv = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(bbv, expr_clone_tree(v));
    expr_variadic_node_prepend_child(bbv, bb);
    expr_variadic_node_prepend_child(sum, bbv);

    void* expanded_expr = expr_expand(sum);
    expr_free_tree(sum);
    void* reduced_expr = expr_reduce(expanded_expr);
    expr_free_tree(expanded_expr);
    return reduced_expr;
}

void* func_rotate_vector_in_plane(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 3)
    {
        struct expr_node* child = var_node->head;
        void* v = child;

        child = child->next;
        void* B = child;

        child = child->next;
        f64 theta = 0;
        if (expr_is_instantiated_scalar_node(child))
        {
            theta = ((struct leaf_node*)child)->f64_value;
        }

        return expr_rotate_vector_in_plane(v, B, theta);
    }

    return expr_clone_tree(func);
}
