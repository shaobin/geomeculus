/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <math.h>

#include "expr.h"

/*
 * The exponential of multivector that is of dimension 3 is another multivector of dimension 3.
 * Ref: Exponentials of general multivectors in 3D Clifford algebras, theorem 2
 */
void* expr_exp_mv3(f64 a0, f64 a1, f64 a2, f64 a3, f64 a12, f64 a13, f64 a23, f64 a123)
{
    f64 exp_a0 = exp(a0);

    f64 b0 = 0;
    f64 b123 = 0;

    f64 a_S = a1 * a1 + a2 * a2 + a3 * a3 - a12 * a12 - a13 * a13 - a23 * a23;
    f64 a_I = 2 * (a3 * a12 - a2 * a13 + a1 * a23);

    if (a_S == 0 && a_I == 0)
    {
        // For this case, the code implemented below following the algorithm provided in the reference paper doesn't seem right
        // an example: e1 + e1 * e2, the algorithm will give output 1, which is wrong
        // We return the result of expr_exponential(expr) instead

        /*
        b0 = cos(a123);
        b123 = sin(a123);

        void* node_b0 = expr_create_scalar_node_with_f64(exp_a0 * b0);

        void* node_b123 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_b123, expr_create_scalar_node_with_f64(exp_a0 * b123));
        expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(1));
        expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(2));
        expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(3));

        void* result = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(result, node_b0);
        expr_variadic_node_append_child(result, node_b123);

        void* reduced = expr_reduce(result);
        expr_free_tree(result);

        return reduced;
        */

        void* node_a0 = expr_create_scalar_node_with_f64(a0);

        void* node_a1 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a1, expr_create_scalar_node_with_f64(a1));
        expr_variadic_node_append_child(node_a1, expr_create_basis_vector_node(1));

        void* node_a2 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a2, expr_create_scalar_node_with_f64(a2));
        expr_variadic_node_append_child(node_a2, expr_create_basis_vector_node(2));

        void* node_a3 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a3, expr_create_scalar_node_with_f64(a3));
        expr_variadic_node_append_child(node_a3, expr_create_basis_vector_node(3));

        void* node_a12 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a12, expr_create_scalar_node_with_f64(a12));
        expr_variadic_node_append_child(node_a12, expr_create_basis_vector_node(1));
        expr_variadic_node_append_child(node_a12, expr_create_basis_vector_node(2));

        void* node_a13 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a13, expr_create_scalar_node_with_f64(a13));
        expr_variadic_node_append_child(node_a13, expr_create_basis_vector_node(1));
        expr_variadic_node_append_child(node_a13, expr_create_basis_vector_node(3));

        void* node_a23 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a23, expr_create_scalar_node_with_f64(a23));
        expr_variadic_node_append_child(node_a23, expr_create_basis_vector_node(2));
        expr_variadic_node_append_child(node_a23, expr_create_basis_vector_node(3));

        void* node_a123 = expr_create_geometric_product_node();
        expr_variadic_node_append_child(node_a123, expr_create_scalar_node_with_f64(a123));
        expr_variadic_node_append_child(node_a123, expr_create_basis_vector_node(1));
        expr_variadic_node_append_child(node_a123, expr_create_basis_vector_node(2));
        expr_variadic_node_append_child(node_a123, expr_create_basis_vector_node(3));

        void* expr = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(expr, node_a0);
        expr_variadic_node_append_child(expr, node_a1);
        expr_variadic_node_append_child(expr, node_a2);
        expr_variadic_node_append_child(expr, node_a3);
        expr_variadic_node_append_child(expr, node_a12);
        expr_variadic_node_append_child(expr, node_a13);
        expr_variadic_node_append_child(expr, node_a23);
        expr_variadic_node_append_child(expr, node_a123);

        void* simp = expr_reduce(expr);
        expr_free_tree(expr);

        void* result = expr_exponential(simp);
        expr_free_tree(simp);

        return result;
    }

    f64 c_norm = sqrt(a_S * a_S + a_I * a_I);

    f64 a_plus = 0; // a_I == 0 && a_S < 0

    if (a_I == 0 && a_S > 0)
    {
        a_plus = sqrt(a_S);
    }
    else if (a_I != 0)
    {
        a_plus = sqrt((a_S + c_norm) * 0.5);
    }

    f64 a_minus = 0; // a_I == 0 && a_S > 0

    if (a_I == 0 && a_S < 0)
    {
        a_minus = sqrt(-a_S);
    }
    else if (a_I != 0)
    {
        a_minus = a_I / sqrt((a_S + c_norm) * 2);
    }

    b0 = cos(a123) * cos(a_minus) * cosh(a_plus) - sin(a123) * sin(a_minus) * sinh(a_plus);

    b123 = sin(a123) * cos(a_minus) * cosh(a_plus) + cos(a123) * sin(a_minus) * sinh(a_plus);

    f64 b1 = cosh(a_plus) * sin(a_minus) * (
        (a_minus * a1 - a_plus * a23) * cos(a123) - (a_plus * a1 + a_minus * a23) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (a_plus * a1 + a_minus * a23) * cos(a123) + (a_minus * a1 - a_plus * a23) * sin(a123)
    );

    f64 b2 = cosh(a_plus) * sin(a_minus) * (
        (a_minus * a2 + a_plus * a13) * cos(a123) + (-a_plus * a2 + a_minus * a13) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (a_plus * a2 - a_minus * a13) * cos(a123) + (a_minus * a2 + a_plus * a13) * sin(a123)
    );

    f64 b3 = cosh(a_plus) * sin(a_minus) * (
        (a_minus * a3 - a_plus * a12) * cos(a123) - (a_plus * a3 + a_minus * a12) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (a_plus * a3 + a_minus * a12) * cos(a123) + (a_minus * a3 - a_plus * a12) * sin(a123)
    );

    f64 b12 = cosh(a_plus) * sin(a_minus) * (
        (a_plus * a3 + a_minus * a12) * cos(a123) + (a_minus * a3 - a_plus * a12) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (-a_minus * a3 + a_plus * a12) * cos(a123) + (a_plus * a3 + a_minus * a12) * sin(a123)
    );

    f64 b13 = -cosh(a_plus) * sin(a_minus) * (
        (a_plus * a2 - a_minus * a13) * cos(a123) + (a_minus * a2 + a_plus * a13) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (a_minus * a2 + a_plus * a13) * cos(a123) + (-a_plus * a2 + a_minus * a13) * sin(a123)
    );

    f64 b23 = cosh(a_plus) * sin(a_minus) * (
        (a_plus * a1 + a_minus * a23) * cos(a123) + (a_minus * a1 - a_plus * a23) * sin(a123)
    ) + sinh(a_plus) * cos(a_minus) * (
        (-a_minus * a1 + a_plus * a23) * cos(a123) + (a_plus * a1 + a_minus * a23) * sin(a123)
    );

    void* node_b0 = expr_create_scalar_node_with_f64(exp_a0 * b0);
    void* node_b1 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b1, expr_create_scalar_node_with_f64(exp_a0 * b1 / c_norm));
    expr_variadic_node_append_child(node_b1, expr_create_basis_vector_node(1));

    void *node_b2 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b2, expr_create_scalar_node_with_f64(exp_a0 * b2 / c_norm));
    expr_variadic_node_append_child(node_b2, expr_create_basis_vector_node(2));

    void *node_b3 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b3, expr_create_scalar_node_with_f64(exp_a0 * b3 / c_norm));
    expr_variadic_node_append_child(node_b3, expr_create_basis_vector_node(3));

    void *node_b12 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b12, expr_create_scalar_node_with_f64(exp_a0 * b12 / c_norm));
    expr_variadic_node_append_child(node_b12, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(node_b12, expr_create_basis_vector_node(2));

    void *node_b13 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b13, expr_create_scalar_node_with_f64(exp_a0 * b13 / c_norm));
    expr_variadic_node_append_child(node_b13, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(node_b13, expr_create_basis_vector_node(3));

    void *node_b23 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b23, expr_create_scalar_node_with_f64(exp_a0 * b23 / c_norm));
    expr_variadic_node_append_child(node_b23, expr_create_basis_vector_node(2));
    expr_variadic_node_append_child(node_b23, expr_create_basis_vector_node(3));

    void* node_b123 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(node_b123, expr_create_scalar_node_with_f64(exp_a0 * b123));
    expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(2));
    expr_variadic_node_append_child(node_b123, expr_create_basis_vector_node(3));

    void* result = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(result, node_b0);
    expr_variadic_node_append_child(result, node_b1);
    expr_variadic_node_append_child(result, node_b2);
    expr_variadic_node_append_child(result, node_b3);
    expr_variadic_node_append_child(result, node_b12);
    expr_variadic_node_append_child(result, node_b13);
    expr_variadic_node_append_child(result, node_b23);
    expr_variadic_node_append_child(result, node_b123);

    void* reduced = expr_reduce(result);
    expr_free_tree(result);

    return reduced;
}

static void expr_exp_3d_get_coeffs(void* expr, f64* args)
{
    void* simplified = expr_evaluate(expr, NULL);

    if (expr_is_instantiated_scalar_node(simplified))
    {
        struct leaf_node* scalar = simplified;
        expr_free_tree(simplified);
        args[0] += scalar->f64_value;
        return;
    }

    if (expr_is_basis_vector_node(simplified))
    {
        struct leaf_node* basis_vector = simplified;

        u64 dim_idx = basis_vector->dim_index;

        if (dim_idx == 1)
        {
            expr_free_tree(simplified);
            args[1] += 1;
            return;
        }
        if (dim_idx == 2)
        {
            expr_free_tree(simplified);
            args[2] += 1;
            return;
        }
        if (dim_idx == 3)
        {
            expr_free_tree(simplified);
            args[3] += 1;
            return;
        }

        printf("expr_exp: basis vector index out of range\n");
        expr_free_tree(simplified);
        return;
    }

    if (expr_is_geometric_product_node(simplified))
    {
        struct variadic_node* var_node = simplified;

        void* child = var_node->head;
        u64 child_count = var_node->length;

        if (child_count == 1)
        {
            expr_exp_3d_get_coeffs(child, args);
            expr_free_tree(simplified);
            return;
        }

        if (child_count == 2)
        {
            void* child1 = child;
            void* child2 = ((struct expr_node*)child)->next;

            if (expr_is_scalar_node(child1) && expr_is_basis_vector_node(child2))
            {
                struct leaf_node* scalar = child1;
                struct leaf_node* basis_vector = child2;

                u64 dim_idx = basis_vector->dim_index;

                if (dim_idx == 1)
                {
                    expr_free_tree(simplified);
                    args[1] += scalar->f64_value;
                    return;
                }
                if (dim_idx == 2)
                {
                    expr_free_tree(simplified);
                    args[2] += scalar->f64_value;
                    return;
                }
                if (dim_idx == 3)
                {
                    expr_free_tree(simplified);
                    args[3] += scalar->f64_value;
                    return;
                }

                printf("expr_exp: basis vector index out of range\n");
                expr_free_tree(simplified);
                return;
            }

            if (expr_is_basis_vector_node(child1) && expr_is_basis_vector_node(child2))
            {
                struct leaf_node* basis_vector1 = child1;
                struct leaf_node* basis_vector2 = child2;

                u64 dim_idx1 = basis_vector1->dim_index;
                u64 dim_idx2 = basis_vector2->dim_index;

                if (dim_idx1 == 1 && dim_idx2 == 2)
                {
                    expr_free_tree(simplified);
                    args[4] += 1;
                    return;
                }
                if (dim_idx1 == 1 && dim_idx2 == 3)
                {
                    expr_free_tree(simplified);
                    args[5] += 1;
                    return;
                }
                if (dim_idx1 == 2 && dim_idx2 == 3)
                {
                    expr_free_tree(simplified);
                    args[6] += 1;
                    return;
                }

                printf("expr_exp: basis vector index out of range\n");
                expr_free_tree(simplified);
                return;
            }

            printf("expr_exp: unsupported expression\n");
            expr_free_tree(simplified);
            return;
        }

        if (child_count == 3)
        {
            void* child1 = child;
            void* child2 = ((struct expr_node*)child)->next;
            void* child3 = ((struct expr_node*)child2)->next;

            if (expr_is_scalar_node(child1) && expr_is_basis_vector_node(child2) && expr_is_basis_vector_node(child3))
            {
                struct leaf_node* scalar = child1;
                struct leaf_node* basis_vector1 = child2;
                struct leaf_node* basis_vector2 = child3;

                u64 dim_idx1 = basis_vector1->dim_index;
                u64 dim_idx2 = basis_vector2->dim_index;

                if (dim_idx1 == 1 && dim_idx2 == 2)
                {
                    expr_free_tree(simplified);
                    args[4] += scalar->f64_value;
                    return;
                }
                if (dim_idx1 == 1 && dim_idx2 == 3)
                {
                    expr_free_tree(simplified);
                    args[5] += scalar->f64_value;
                    return;
                }
                if (dim_idx1 == 2 && dim_idx2 == 3)
                {
                    expr_free_tree(simplified);
                    args[6] += scalar->f64_value;
                    return;
                }

                printf("expr_exp: basis vector index out of range\n");
                expr_free_tree(simplified);
                return;
            }

            if (expr_is_basis_vector_node(child1) && expr_is_basis_vector_node(child2) && expr_is_basis_vector_node(child3))
            {
                struct leaf_node* basis_vector1 = child1;
                struct leaf_node* basis_vector2 = child2;
                struct leaf_node* basis_vector3 = child3;

                u64 dim_idx1 = basis_vector1->dim_index;
                u64 dim_idx2 = basis_vector2->dim_index;
                u64 dim_idx3 = basis_vector3->dim_index;

                if (dim_idx1 == 1 && dim_idx2 == 2 && dim_idx3 == 3)
                {
                    expr_free_tree(simplified);
                    args[7] += 1;
                    return;
                }

                printf("expr_exp: basis vector index out of range\n");
                expr_free_tree(simplified);
                return;
            }

            printf("expr_exp: unsupported expression\n");
            expr_free_tree(simplified);
            return;
        }

        if (child_count == 4)
        {
            void* child1 = child;
            void* child2 = ((struct expr_node*)child)->next;
            void* child3 = ((struct expr_node*)child2)->next;
            void* child4 = ((struct expr_node*)child3)->next;

            if (expr_is_scalar_node(child1) && expr_is_basis_vector_node(child2) && expr_is_basis_vector_node(child3) && expr_is_basis_vector_node(child4))
            {
                struct leaf_node* scalar = child1;
                struct leaf_node* basis_vector1 = child2;
                struct leaf_node* basis_vector2 = child3;
                struct leaf_node* basis_vector3 = child4;

                u64 dim_idx1 = basis_vector1->dim_index;
                u64 dim_idx2 = basis_vector2->dim_index;
                u64 dim_idx3 = basis_vector3->dim_index;

                if (dim_idx1 == 1 && dim_idx2 == 2 && dim_idx3 == 3)
                {
                    expr_free_tree(simplified);
                    args[7] += scalar->f64_value;
                    return;
                }

                printf("expr_exp: basis vector index out of range\n");
                expr_free_tree(simplified);
                return;
            }

            printf("expr_exp: unsupported expression\n");
            expr_free_tree(simplified);
            return;
        }

        printf("expr_exp: unsupported expression\n");
        expr_free_tree(simplified);
        return;
    }

    if (expr_is_geometric_sum_node(expr))
    {
        struct variadic_node* var_node = expr;
        void* child = var_node->head;
        u64 child_count = var_node->length;

        for (u64 i = 0; i < child_count; ++i)
        {
            expr_exp_3d_get_coeffs(child, args);
            child = ((struct expr_node*)child)->next;
        }

        expr_free_tree(simplified);
        return;
    }

    printf("expr_exp: unsupported expression\n");
    expr_free_tree(simplified);
}

void* expr_exp_3d(void* expr)
{
    f64 args[8] = {0};
    expr_exp_3d_get_coeffs(expr, args);
    return expr_exp_mv3(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
}

void* func_exp_3d(void* func_node)
{
    struct variadic_node* var_node = (struct variadic_node*)func_node;
    if (var_node->length == 1)
    {
        return expr_exp_3d(var_node->head);
    }

    return expr_clone_tree(func_node);
}
