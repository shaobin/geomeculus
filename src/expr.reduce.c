/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "mm.h"
#include "runtime.h"
#include "bitmap.h"
#include "list.h"
#include "b64_kv_array.h"
#include "id_signature.h"
#include "config.h"

#include "expr.reduce.product.c.inc"
#include "expr.reduce.sum.c.inc"

static void* expr_reduce_function_node(void* node)
{
    struct variadic_node* var_node = node;
    if (var_node->length == 0)
    {
        return expr_clone_tree(node);
    }

    struct variadic_node* reduced_node = expr_create_function_node_with_symbol(var_node->symbol);
    reduced_node->type = var_node->type;

    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        struct expr_node* reduced_child = expr_reduce(child);
        expr_variadic_node_append_child(reduced_node, reduced_child);

        child = child->next;
    }

    return reduced_node;
}

void* expr_reduce(void* node)
{
    if (expr_is_geometric_sum_node(node))
    {
        return expr_reduce_sum_node(node);
    }
    else if (expr_is_geometric_product_node(node))
    {
        return expr_reduce_product_node(node);
    }
    else if (expr_is_function_node(node))
    {
        return expr_reduce_function_node(node);
    }
    else
    {
        return expr_clone_tree(node);
    }
}

void* func_reduce(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_reduce(var_node->head);
    }

    return expr_clone_tree(func);
}

#ifdef MODULE_TEST
#include <math.h>
#include <stdio.h>

#include "mm.h"
#include "cli_args.h"

static u64 force_run_all_tests = 0;

static u64 bubble_sort_basis_vectors(void* pbasis_vectors, u64 num_basis_vectors)
{
    u64* dim_idxes = x_malloc(num_basis_vectors * sizeof(u64));
    struct leaf_node* child = (struct leaf_node*)pbasis_vectors;
    for (u64 i = 0; i < num_basis_vectors; ++i)
    {
        dim_idxes[i] = child->dim_index;
        child = child->next;
    }

    u64 swap_count = bubble_sort_dim_indexes(dim_idxes, num_basis_vectors);

    child = (struct leaf_node*)pbasis_vectors;
    for (u64 i = 0; i < num_basis_vectors; ++i)
    {
        child->dim_index = dim_idxes[i];
        child = child->next;
    }

    x_free(dim_idxes);

    return swap_count;
}

static void test_bubble_sort_basis_vectors(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    char* json_str = "{'*': ['e_2','e_1', 'e_3', 'e_0']}";

    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    printf("\nbefore sorting:\n");
    u64 swap_count = bubble_sort_basis_vectors(((struct variadic_node*)expr_node)->head, ((struct variadic_node*)expr_node)->length);
    printf("\nswap count: %lu\n", swap_count);

    printf("\nafter sorting:\n");
    expr_print_infix_with_label(expr_node, "infix expr");

    expr_free_tree(expr_node);
    printf("\n");
}

static void test_variable_blade_signature(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct variadic_node* gp = expr_create_geometric_product_node();
    expr_variadic_node_append_child(gp, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(gp, expr_create_scalar_variable_node(2));

    struct variadic_node* reciprocal_node = expr_create_function_node("reciprocal");
    expr_variadic_node_append_child(reciprocal_node, expr_create_scalar_node_with_f64(4.0));
    expr_variadic_node_append_child(gp, reciprocal_node);

    expr_variadic_node_append_child(gp, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(gp, expr_create_scalar_variable_node(3));
    expr_variadic_node_prepend_child(gp, expr_create_scalar_node_with_f64(2.71828));

    expr_print_tree_with_label(gp, "gp");

    f64 coeff = NAN;
    struct id_signature* sig = variable_blade_signature(gp, &coeff);
    expr_free_tree(gp);

    printf("coeff: %f\n", coeff);
    id_signature_print(sig);

    void* scalar_terms = create_scalar_terms_from_signature(sig, coeff);
    id_signature_free(sig);

    expr_print_tree_with_label(scalar_terms, "scalar_terms");
    expr_free_tree(scalar_terms);
}

int main(int argc, char* argv[])
{
    runtime_initialize();
    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_bubble_sort_basis_vectors(0);

    test_variable_blade_signature(1);

    runtime_finalize();
    mm_print_status();
    return 0;
}
#endif
