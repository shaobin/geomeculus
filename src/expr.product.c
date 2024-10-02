/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "expr.h"
#include "list.h"
#include "mm.h"
#include "cartesian_product.h"

void* expr_geometric_product(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, expr_clone_tree(A));
    expr_variadic_node_append_child(expr, expr_clone_tree(B));

    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);
    return simplified_expr;
}

/*
 * A simplified term is a geometric product of basis vectors and constants.
 * Ref: Clifford algebra to geometric calculus, p6, eq 1.21a
 */
void* expr_inner_product_of_simplied_terms(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    u64 grade_A = expr_grade_of_simplified_term(A);
    u64 grade_B = expr_grade_of_simplified_term(B);

    // eq 1.21b
    if (!grade_A || !grade_B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    i64 grade_A_minus_B = grade_A - grade_B;
    u64 k = llabs(grade_A_minus_B);

    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, expr_clone_tree(A));
    expr_variadic_node_append_child(expr, expr_clone_tree(B));
    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);

    void* k_vector = expr_k_vector_of_simplified_expr(simplified_expr, k);
    expr_free_tree(simplified_expr);

    if (!k_vector)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    return k_vector;
}

/*
 * Ref: Clifford algebra to geometric calculus, p6
 */
void* expr_inner_product(struct list* exprs)
{
    if (exprs->length == 1)
    {
        return expr_clone_tree(exprs->head->data);
    }

    struct list* simplified_exprs = list_create();

    struct list* terms = list_create();
    struct list_node* expr_node = exprs->head;
    for (u64 i = 0; i < exprs->length; ++i)
    {
        void* expr = expr_expand_reduce(expr_node->data);
        list_append(simplified_exprs, expr);
        struct list* children = list_create();
        if (expr_is_geometric_sum_node(expr))
        {
            struct variadic_node* var_node = expr;
            struct expr_node* child = var_node->head;
            for (u64 j = 0; j < var_node->length; ++j)
            {
                list_append(children, child);
                child = child->next;
            }
        }
        else
        {
            list_append(children, expr);
        }
        list_append(terms, children);

        expr_node = expr_node->next;
    }

    struct list* expanded_terms = cartesian_product(terms);

    // free terms
    struct list_node* terms_listnode = terms->head;
    while(terms_listnode != NULL)
    {
        struct list* term_list = (struct list*)terms_listnode->data;
        list_free(term_list, 0);
        terms_listnode = terms_listnode->next;
    }
    list_free(terms, 0);

    struct variadic_node* expanded_expr = expr_create_geometric_sum_node();
    struct list_node* expanded_terms_listnode = expanded_terms->head;
    for (u64 i = 0; i < expanded_terms->length; ++i)
    {
        struct list* expanded_product_term = expanded_terms_listnode->data;
        struct list_node* product_term_child = expanded_product_term->head;
        void* tmp_inner_product_1 = product_term_child->data;
        for (u64 j = 1; j < expanded_product_term->length; ++j)
        {
            product_term_child = product_term_child->next;

            void* tmp_inner_product_2 = expr_inner_product_of_simplied_terms(tmp_inner_product_1, product_term_child->data);
            if (j > 1)
            {
                expr_free_tree(tmp_inner_product_1);
            }
            tmp_inner_product_1 = tmp_inner_product_2;
        }
        expr_variadic_node_append_child(expanded_expr, tmp_inner_product_1);

        expanded_terms_listnode = expanded_terms_listnode->next;
    }

    // free simplified_exprs
    struct list_node* simplified_exprs_listnode = simplified_exprs->head;
    while(simplified_exprs_listnode != NULL)
    {
        void* simplified_expr = simplified_exprs_listnode->data;
        expr_free_tree(simplified_expr);
        simplified_exprs_listnode = simplified_exprs_listnode->next;
    }
    list_free(simplified_exprs, 0);

    // free expanded_terms
    struct list_node* expanded_terms_listnode2 = expanded_terms->head;
    while(expanded_terms_listnode2 != NULL)
    {
        struct list* expanded_product_term = (struct list*)expanded_terms_listnode2->data;
        list_free(expanded_product_term, 0);
        expanded_terms_listnode2 = expanded_terms_listnode2->next;
    }
    list_free(expanded_terms, 0);

    void* simplified_inner_product = expr_expand_reduce(expanded_expr);
    expr_free_tree(expanded_expr);

    return simplified_inner_product;
}

void* func_inner_product(void* func)
{
    struct list* exprs = list_create();
    struct variadic_node* var_node = (struct variadic_node*)func;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        list_append(exprs, child);
        child = child->next;
    }

    void* inner_product = expr_inner_product(exprs);
    list_free(exprs, 0);
    return inner_product;
}

/*
 * A simplified term is a geometric product of basis vectors and constants.
 * Ref: Clifford algebra to geometric calculus, p6, eq 1.22a
 */
void* expr_outer_product_of_simplied_terms(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    u64 grade_A = expr_grade_of_simplified_term(A);
    u64 grade_B = expr_grade_of_simplified_term(B);

    u64 k = grade_A + grade_B;

    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, expr_clone_tree(A));
    expr_variadic_node_append_child(expr, expr_clone_tree(B));
    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);

    void* k_vector = expr_k_vector_of_simplified_expr(simplified_expr, k);
    expr_free_tree(simplified_expr);

    if (!k_vector)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    return k_vector;
}

/*
 * Ref: Clifford algebra to geometric calculus, p6
 */
void* expr_outer_product(struct list* exprs)
{
    if (exprs->length == 1)
    {
        return expr_clone_tree(exprs->head->data);
    }

    struct list* simplified_exprs = list_create();

    struct list* terms = list_create();
    struct list_node* expr_node = exprs->head;
    for (u64 i = 0; i < exprs->length; ++i)
    {
        void* expr = expr_expand_reduce(expr_node->data);
        list_append(simplified_exprs, expr);
        struct list* children = list_create();
        if (expr_is_geometric_sum_node(expr))
        {
            struct variadic_node* var_node = expr;
            struct expr_node* child = var_node->head;
            for (u64 j = 0; j < var_node->length; ++j)
            {
                list_append(children, child);
                child = child->next;
            }
        }
        else
        {
            list_append(children, expr);
        }
        list_append(terms, children);

        expr_node = expr_node->next;
    }

    struct list* expanded_terms = cartesian_product(terms);

    // free terms
    struct list_node* terms_listnode = terms->head;
    while(terms_listnode != NULL)
    {
        struct list* term_list = (struct list*)terms_listnode->data;
        list_free(term_list, 0);
        terms_listnode = terms_listnode->next;
    }
    list_free(terms, 0);

    struct variadic_node* expanded_expr = expr_create_geometric_sum_node();
    struct list_node* expanded_terms_listnode = expanded_terms->head;
    for (u64 i = 0; i < expanded_terms->length; ++i)
    {
        struct list* expanded_product_term = expanded_terms_listnode->data;
        struct list_node* product_term_child = expanded_product_term->head;
        void* tmp_inner_product_1 = product_term_child->data;
        for (u64 j = 1; j < expanded_product_term->length; ++j)
        {
            product_term_child = product_term_child->next;

            void* tmp_inner_product_2 = expr_outer_product_of_simplied_terms(tmp_inner_product_1, product_term_child->data);
            if (j > 1)
            {
                expr_free_tree(tmp_inner_product_1);
            }
            tmp_inner_product_1 = tmp_inner_product_2;
        }
        expr_variadic_node_append_child(expanded_expr, tmp_inner_product_1);

        expanded_terms_listnode = expanded_terms_listnode->next;
    }

    // free simplified_exprs
    struct list_node* simplified_exprs_listnode = simplified_exprs->head;
    while(simplified_exprs_listnode != NULL)
    {
        void* simplified_expr = simplified_exprs_listnode->data;
        expr_free_tree(simplified_expr);
        simplified_exprs_listnode = simplified_exprs_listnode->next;
    }
    list_free(simplified_exprs, 0);

    // free expanded_terms
    struct list_node* expanded_terms_listnode2 = expanded_terms->head;
    while(expanded_terms_listnode2 != NULL)
    {
        struct list* expanded_product_term = (struct list*)expanded_terms_listnode2->data;
        list_free(expanded_product_term, 0);
        expanded_terms_listnode2 = expanded_terms_listnode2->next;
    }
    list_free(expanded_terms, 0);

    void* simplified_outer_product = expr_expand_reduce(expanded_expr);
    expr_free_tree(expanded_expr);

    return simplified_outer_product;
}

void* func_outer_product(void* func)
{
    struct list* exprs = list_create();
    struct variadic_node* var_node = (struct variadic_node*)func;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        list_append(exprs, child);
        child = child->next;
    }

    void* outer_product = expr_outer_product(exprs);
    list_free(exprs, 0);
    return outer_product;
}

void* expr_scalar_product(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, expr_clone_tree(A));
    expr_variadic_node_append_child(expr, expr_clone_tree(B));
    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);

    void* scalar_expr = expr_k_vector_of_simplified_expr(simplified_expr, 0);
    void* simplified_scalar_expr = expr_expand_reduce(scalar_expr);

    expr_free_tree(simplified_expr);
    expr_free_tree(scalar_expr);

    return simplified_scalar_expr;
}

void* func_scalar_product(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* A = child;

        child = child->next;
        void* B = child;

        return expr_scalar_product(A, B);
    }

    return expr_clone_tree(func);
}

/*
 * 0.5 * (AB - BA)
 * Ref: Clifford algebra to geometric calculus, p14
 */
void* expr_commutator_product(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    void* AB = expr_create_geometric_product_node();
    expr_variadic_node_append_child(AB, expr_create_scalar_node_with_f64(0.5));
    expr_variadic_node_append_child(AB, expr_clone_tree(A));
    expr_variadic_node_append_child(AB, expr_clone_tree(B));

    void* BA = expr_create_geometric_product_node();
    expr_variadic_node_append_child(BA, expr_create_scalar_node_with_f64(-0.5));
    expr_variadic_node_append_child(BA, expr_clone_tree(B));
    expr_variadic_node_append_child(BA, expr_clone_tree(A));

    void* expr = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expr, AB);
    expr_variadic_node_append_child(expr, BA);

    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);

    return simplified_expr;
}

void* func_commutator_product(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* A = child;

        child = child->next;
        void* B = child;

        return expr_commutator_product(A, B);
    }

    return expr_clone_tree(func);
}

void* expr_anticommutator_product(void* A, void* B)
{
    if (!A || !B)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    void* AB = expr_create_geometric_product_node();
    expr_variadic_node_append_child(AB, expr_create_scalar_node_with_f64(0.5));
    expr_variadic_node_append_child(AB, expr_clone_tree(A));
    expr_variadic_node_append_child(AB, expr_clone_tree(B));

    void* BA = expr_create_geometric_product_node();
    expr_variadic_node_append_child(BA, expr_create_scalar_node_with_f64(0.5));
    expr_variadic_node_append_child(BA, expr_clone_tree(B));
    expr_variadic_node_append_child(BA, expr_clone_tree(A));

    void* expr = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expr, AB);
    expr_variadic_node_append_child(expr, BA);

    void* simplified_expr = expr_expand_reduce(expr);
    expr_free_tree(expr);

    return simplified_expr;
}

void* func_anticommutator_product(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* child = var_node->head;
        void* A = child;

        child = child->next;
        void* B = child;

        return expr_anticommutator_product(A, B);
    }

    return expr_clone_tree(func);
}
