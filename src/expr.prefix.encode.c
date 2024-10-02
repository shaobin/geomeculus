/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "expr.h"
#include "mm.h"

static u64 expr_prefix(void* node, char* buffer, u64* idx, u64 display_mode)
{
    if (buffer == NULL)
    {
        if (expr_is_leaf_node(node))
        {
            u64 bufsz = expr_encode_leaf_node(node, NULL, idx, display_mode);
            return bufsz;
        }

        u64 bufsz = 1; // strlen("(");
        struct variadic_node* var_node = node;
        if (expr_is_geometric_product_node(node) || expr_is_geometric_sum_node(node))
        {
            bufsz += 1; // strlen("*"); or strlen("+");
        }
        else if (expr_is_function_node(node))
        {
            bufsz += strlen(var_node->symbol->name);
        }

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            bufsz += 1; // strlen(" ");
            bufsz += expr_prefix(child, NULL, idx, display_mode);
            child = ((struct expr_node*)child)->next;
        }
        bufsz += strlen(")");
        return bufsz;
    }

    if (expr_is_leaf_node(node))
    {
        u64 bufsz = expr_encode_leaf_node(node, buffer, idx, display_mode);
        return bufsz;
    }

    struct variadic_node* var_node = node;
    *idx += sprintf(buffer + *idx, "(");
    if (expr_is_geometric_product_node(node))
    {
        *idx += sprintf(buffer + *idx, "*");
    }
    else if (expr_is_geometric_sum_node(node))
    {
        *idx += sprintf(buffer + *idx, "+");
    }
    else if (expr_is_function_node(node))
    {
        *idx += sprintf(buffer + *idx, "%s", var_node->symbol->name);
    }

    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        *idx += sprintf(buffer + *idx, " ");
        expr_prefix(child, buffer, idx, display_mode);
        child = child->next;
    }

    *idx += sprintf(buffer + *idx, ")");

    return 0;
}

char* expr_encode_prefix(void* node, u64 display_mode)
{
    if (node == NULL)
    {
        return "null";
    }

    u64 bufsz = expr_prefix(node, NULL, NULL, display_mode);
    char* buffer = x_malloc(bufsz + 1);
    buffer[bufsz] = '\0';

    u64 idx = 0;
    expr_prefix(node, buffer, &idx, display_mode);

    return buffer;
}
