/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"
#include "mm.h"
#include "utf8.h"
#include "json.h"
#include "list.h"
#include "stack.h"
#include "func_repo.h"

static void encode_expr_to_json(void* expr, char* buffer, u64* idx)
{
    if (buffer == NULL)
    {
        if (expr_is_scalar_variable_node(expr))
        {
            struct leaf_node* scalar = (struct leaf_node*)expr;
            *idx += snprintf(NULL, 0, "\"v%lu\"", scalar->symbol->u64_value);
            return;
        }

        if (expr_is_instantiated_scalar_node(expr))
        {
            struct leaf_node* scalar = (struct leaf_node*)expr;
            *idx += snprintf(NULL, 0, "\"%.16e\"", scalar->f64_value); // we can use %.16e to recover the original f64 value
            return;
        }

        if (expr_is_basis_vector_node(expr))
        {
            struct leaf_node* bv = (struct leaf_node*)expr;
            *idx += snprintf(NULL, 0, "\"e%lu\"", bv->dim_index);
            return;
        }

        if (expr_is_variadic_node(expr))
        {
            if(expr_is_geometric_product_node(expr))
            {
                *idx += snprintf(NULL, 0, "{\"*\": [");
            }
            else if (expr_is_geometric_sum_node(expr))
            {
                *idx += snprintf(NULL, 0, "{\"+\": [");
            }
            else if (expr_is_function_node(expr))
            {
                struct variadic_node* var_node = expr;
                *idx += snprintf(NULL, 0, "{\"%s\": [", var_node->symbol->name);
            }
            else
            {
                *idx += snprintf(NULL, 0, "{\"?\": [");
            }

            struct variadic_node* var_node = (struct variadic_node*)expr;
            void* child = var_node->head;
            encode_expr_to_json(child, NULL, idx);
            for (u64 i = 0; i < var_node->length - 1; ++i)
            {
                child = ((struct expr_node*)child)->next;
                *idx += snprintf(NULL, 0, ", ");
                encode_expr_to_json(child, NULL, idx);
            }
            *idx += snprintf(NULL, 0, "]}");
            return;
        }
    }

    if (expr_is_scalar_variable_node(expr))
    {
        struct leaf_node* scalar = (struct leaf_node*)expr;
        *idx += sprintf(buffer + *idx, "\"v%lu\"", scalar->symbol->u64_value);
        return;
    }

    if (expr_is_instantiated_scalar_node(expr))
    {
        struct leaf_node* scalar = (struct leaf_node*)expr;
        *idx += sprintf(buffer + *idx, "\"%.16e\"", scalar->f64_value); // we can use %.16e to recover the original f64 value
        return;
    }

    if (expr_is_basis_vector_node(expr))
    {
        struct leaf_node* bv = (struct leaf_node*)expr;
        *idx += sprintf(buffer + *idx, "\"e%lu\"", bv->dim_index);
        return;
    }

    if (expr_is_variadic_node(expr))
    {
        if(expr_is_geometric_product_node(expr))
        {
            *idx += sprintf(buffer + *idx, "{\"*\": [");
        }
        else if (expr_is_geometric_sum_node(expr))
        {
            *idx += sprintf(buffer + *idx, "{\"+\": [");
        }
        else if (expr_is_function_node(expr))
        {
            struct variadic_node* var_node = expr;
            *idx += sprintf(buffer + *idx, "{\"%s\": [", var_node->symbol->name);
        }
        else
        {
            *idx += sprintf(buffer + *idx, "{\"?\": [");
        }

        struct variadic_node* var_node = (struct variadic_node*)expr;
        void* child = var_node->head;
        encode_expr_to_json(child, buffer, idx);
        for (u64 i = 0; i < var_node->length - 1; ++i)
        {
            child = ((struct expr_node*)child)->next;
            *idx += sprintf(buffer + *idx, ", ");
            encode_expr_to_json(child, buffer, idx);
        }
        *idx += sprintf(buffer + *idx, "]}");
    }
}

char* expr_encode_to_json_str(void* expr)
{
    u64 idx = 0;
    encode_expr_to_json(expr, NULL, &idx);
    char* buffer = x_malloc(idx + 1);
    idx = 0;
    encode_expr_to_json(expr, buffer, &idx);
    buffer[idx] = '\0';
    return buffer;
}
