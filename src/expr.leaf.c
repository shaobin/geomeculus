/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "expr.h"
#include "mm.h"
#include "string_util.h"

u64 expr_encode_leaf_node(void* node, char* buffer, u64* idx, u64 display_mode)
{
    u64 encoding_precision = f64_representation_precision;
    if (display_mode)
    {
        encoding_precision = f64_display_precision;
    }

    char const* fmt = "%%.%luf";
    int display_format_len = snprintf(NULL, 0, fmt, encoding_precision);
    char f64_fmt[display_format_len + 1];
    snprintf(f64_fmt, display_format_len + 1, fmt, encoding_precision);

    if (buffer == NULL)
    {
        if (expr_is_scalar_node(node))
        {
            if (expr_is_instantiated_node(node))
            {
                struct leaf_node* c = (struct leaf_node*)node;
                int bufsz = snprintf(NULL, 0, f64_fmt, c->f64_value);
                return (u64)bufsz;
            }
            else if (expr_is_variable_node(node))
            {
                struct leaf_node* c = (struct leaf_node*)node;

                int bufsz = snprintf(NULL, 0, "v%lu", c->symbol->u64_value);
                return (u64)bufsz;
            }
        }

        if (expr_is_basis_vector_node(node))
        {
            struct leaf_node* bv = (struct leaf_node*)node;
            int bufsz = snprintf(NULL, 0, "e%lu", bv->dim_index);
            return (u64)bufsz;
        }
    }

    if (expr_is_scalar_node(node))
    {
        if (expr_is_instantiated_node(node))
        {
            struct leaf_node* c = (struct leaf_node*)node;
            int bs = sprintf(buffer + *idx, f64_fmt, c->f64_value);
            *idx += bs;
        }
        else if (expr_is_variable_node(node))
        {
            struct leaf_node* c = (struct leaf_node*)node;
            int bs = sprintf(buffer + *idx, "v%lu", c->symbol->u64_value);
            *idx += bs;
        }
    }

    if (expr_is_basis_vector_node(node))
    {
        struct leaf_node* bv = (struct leaf_node*)node;
        int bs = sprintf(buffer + *idx, "e%lu", bv->dim_index);
        *idx += bs;
    }

    return 0;
}

void* expr_decode_leaf_node(char const* node_str)
{
    if (node_str == NULL)
    {
        return NULL;
    }

    u64 char_count = strlen(node_str);

    if (char_count == 0)
    {
        return NULL;
    }

    if (char_count > 1 && node_str[0] == 'e')
    {
        if (!string_is_integer(node_str + 1))
        {
            return NULL;
        }
        char* end;
        u64 dim_index = strtoull(node_str + 1, &end, 10);
        struct leaf_node* bv = expr_create_basis_vector_node(dim_index);
        return bv;
    }

    if (char_count > 1 && node_str[0] == 'v')
    {
        if (!string_is_integer(node_str + 1))
        {
            return NULL;
        }
        char* end;
        u64 var_id = strtoull(node_str + 1, &end, 10);
        struct leaf_node* c = expr_create_scalar_variable_node(var_id);
        return c;
    }

    if (node_str[0] == '-' || (node_str[0] >= '0' && node_str[0] <= '9'))
    {
        char* end;
        f64 f64_value = strtod(node_str, &end);
        struct leaf_node* c = expr_create_scalar_node_with_f64(f64_value);
        return c;
    }

    return NULL;
}
