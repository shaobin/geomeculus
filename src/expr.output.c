/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "expr.h"
#include "mm.h"

static void dfs_callback_print_tree(void* dfs_node, void* data)
{
    struct top_down_dfs_node* _dfs_node = dfs_node;
    u64 indent = _dfs_node->depth * 3;
    printf("%*s", (int)indent, "");

    void* node = _dfs_node->node;

    if (expr_is_leaf_node(node))
    {
        if (expr_is_scalar_node(node))
        {
            if (expr_is_variable_node(node))
            {
                struct leaf_node* c = (struct leaf_node*)node;
                printf("v%lu\n", c->symbol->u64_value);
            }
            else
            {
                char const* fmt = "%%.%luf\n";
                int display_format_len = snprintf(NULL, 0, fmt, f64_display_precision);
                char f64_fmt[display_format_len + 1];
                snprintf(f64_fmt, display_format_len + 1, fmt, f64_display_precision);

                struct leaf_node* c = (struct leaf_node*)node;
                printf(f64_fmt, c->f64_value);
            }
        }

        if (expr_is_basis_vector_node(node))
        {
            struct leaf_node* bv = (struct leaf_node*)node;
            printf("e%lu\n", bv->dim_index);
        }
    }
    else
    {
        struct variadic_node* var_node = (struct variadic_node*)node;
        if (expr_is_geometric_sum_node(node))
        {
            printf("+\n");
        }
        else if (expr_is_geometric_product_node(node))
        {
            printf("*\n");
        }
        else if (expr_is_function_node(node))
        {
            printf("%s\n", var_node->symbol->name);
        }
        else
        {
            printf("{%lu}\n", var_node->type);
        }
    }
}

void expr_print_tree(void* node)
{
    if (node != NULL)
    {
        expr_top_down_dfs(node, NULL, dfs_callback_print_tree);
    }
    else
    {
        printf("null");
    }
}

void expr_print_tree_with_label(void* node, char const* label)
{
    printf("%s:\n", label);
    expr_print_tree(node);
    printf("\n");
}

void expr_print_infix(void* node)
{
    char* buffer = expr_encode_infix(node, 1);
    printf("%s\n", buffer);
    x_free(buffer);
}

void expr_print_infix_with_label(void* node, char* label)
{
    printf("%s: ", label);
    expr_print_infix(node);
}

void expr_print_prefix(void* node)
{
    char* buffer = expr_encode_prefix(node, 1);
    printf("%s\n", buffer);
    x_free(buffer);
}

void expr_print_prefix_with_label(void* node, char* label)
{
    printf("%s: ", label);
    expr_print_prefix(node);
}

void expr_print_json(void* node)
{
    char* buffer = expr_encode_to_json_str(node);
    printf("%s\n", buffer);
    x_free(buffer);
}

void expr_print_json_with_label(void* node, char* label)
{
    printf("%s: ", label);
    expr_print_json(node);
}
