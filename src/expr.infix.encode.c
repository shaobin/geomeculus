/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "expr.h"
#include "mm.h"

static u64 expr_infix(void* node, char* buffer, u64* idx, u64 display_mode)
{
    if (buffer == NULL)
    {
        if (expr_is_leaf_node(node))
        {
            u64 bufsz = expr_encode_leaf_node(node, NULL, idx, display_mode);
            return bufsz;
        }

        char separator[] = " + ";
        u64 bufsz = 0;
        struct variadic_node* var_node = (struct variadic_node*)node;
        if (expr_is_geometric_product_node(node))
        {
            separator[1] = '*';

            bufsz += strlen("(");

            void* child = var_node->head;

            child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                bufsz += expr_infix(child, NULL, idx, display_mode);
                child = ((struct expr_node*)child)->next;
            }

            bufsz += (var_node->length - 1) * strlen(separator);

            bufsz += strlen(")");
            return bufsz;

        }
        else if (expr_is_geometric_sum_node(node))
        {
            bufsz += strlen("(");

            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                bufsz += expr_infix(child, NULL, idx, display_mode);
                child = ((struct expr_node*)child)->next;
            }

            bufsz += (var_node->length - 1) * strlen(separator);

            bufsz += strlen(")");
            return bufsz;
        }
        else if (expr_is_function_node(node))
        {
            char args_separator[] = ", ";
            bufsz += strlen(var_node->symbol->name);
            bufsz += strlen("(");

            void* child = var_node->head;
            for (u64 i = 0; i < var_node->length; ++i)
            {
                bufsz += expr_infix(child, NULL, idx, display_mode);
                child = ((struct expr_node*)child)->next;
            }

            bufsz += (var_node->length - 1) * strlen(args_separator);

            bufsz += strlen(")");
            return bufsz;
        }

        printf("unknown node type: %lu\n", var_node->type);
        return 0;
    }

    if (expr_is_leaf_node(node))
    {
        return expr_encode_leaf_node(node, buffer, idx, display_mode);
    }

    char separator[] = " + ";
    struct variadic_node* var_node = (struct variadic_node*)node;
    if (expr_is_geometric_product_node(node))
    {
        separator[1] = '*';

        memcpy(buffer + *idx, "(", strlen("("));
        *idx += strlen("(");

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            if (i != 0)
            {
                memcpy(buffer + *idx, separator, strlen(separator));
                *idx += strlen(separator);
            }
            expr_infix(child, buffer, idx, display_mode);
            child = ((struct expr_node*)child)->next;
        }

        memcpy(buffer + *idx, ")", strlen(")"));
        *idx += strlen(")");
        return 0;
    }
    else if (expr_is_geometric_sum_node(node))
    {
        memcpy(buffer + *idx, "(", strlen("("));
        *idx += strlen("(");

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            if (i != 0)
            {
                memcpy(buffer + *idx, separator, strlen(separator));
                *idx += strlen(separator);
            }
            expr_infix(child, buffer, idx, display_mode);
            child = ((struct expr_node*)child)->next;
        }

        memcpy(buffer + *idx, ")", strlen(")"));
        *idx += strlen(")");
        return 0;
    }
    else if (expr_is_function_node(node))
    {
        char args_separator[] = ", ";
        memcpy(buffer + *idx, var_node->symbol->name, strlen(var_node->symbol->name));
        *idx += strlen(var_node->symbol->name);
        memcpy(buffer + *idx, "(", strlen("("));
        *idx += strlen("(");

        void* child = var_node->head;
        for (u64 i = 0; i < var_node->length; ++i)
        {
            if (i != 0)
            {
                memcpy(buffer + *idx, args_separator, strlen(args_separator));
                *idx += strlen(args_separator);
            }
            expr_infix(child, buffer, idx, display_mode);
            child = ((struct expr_node*)child)->next;
        }

        memcpy(buffer + *idx, ")", strlen(")"));
        *idx += strlen(")");
        return 0;
    }

    printf("unknown node type +++>: %lu\n", var_node->type);

    return 0;
}

char* expr_encode_infix(void* node, u64 display_mode)
{
    if (node == NULL)
    {
        char* null_buffer = x_malloc(5);
        memcpy(null_buffer, "null", 5);
        return null_buffer;
    }

    u64 bufsz = expr_infix(node, NULL, NULL, display_mode);
    char* buffer = x_malloc(bufsz + 1);
    buffer[bufsz] = '\0';

    u64 idx = 0;
    expr_infix(node, buffer, &idx, display_mode);

    return buffer;
}
