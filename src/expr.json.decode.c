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

static u32 const UTF8_MINUS = 0x2D;
static u32 const UTF8_ZERO = 0x30;
static u32 const UTF8_NINE = 0x39;

/*
Rules: each variadic expr node, "+" or "*", is encoded as a json object with a single key-value pair,
the key is the operator, the value is an array of children.
Sample expr encoded in json format:
{
    "+":
    [
        {
            "*":
            [
                "e_0",
                "e_1"
            ]
        },
        {
            "+":
            [
                "e_2",
                "e_3",
                "3.14159e+00"
            ]
        },
        {
            "*":
            [
                "e_4",
                "e_5"
            ]
        }
    ]
}
*/
void* expr_decode_json(struct json* json)
{
    if (json->type == JSON_VALUE_TYPE_STRING)
    {
        if (json->length > 1 &&  json->codepoints[0] == (u32)'e')
        {
            u64 dim_index = utf8_codepoints_to_u64(json->codepoints + 1, json->length - 1);
            struct leaf_node* bv = expr_create_basis_vector_node(dim_index);
            return bv;
        }

        if (json->length > 1 && json->codepoints[0] == (u32)'v')
        {
            u64 var_id = utf8_codepoints_to_u64(json->codepoints + 1, json->length - 1);
            struct leaf_node* c = expr_create_scalar_variable_node(var_id);
            return c;
        }

        if (json->codepoints[0] == UTF8_MINUS || (json->codepoints[0] >= UTF8_ZERO && json->codepoints[0] <= UTF8_NINE))
        {
            f64 data = utf8_codepoints_to_f64(json->codepoints, json->length);
            struct leaf_node* c = expr_create_scalar_node_with_f64(data);
            return c;
        }
    }

    if (json->type == JSON_VALUE_TYPE_OBJECT)
    {
        if (json->length != 1)
        {
            printf("Invalid json object length: %lu\n", json->length);
            return NULL;
        }

        struct json* operator = json->child;
        void* operator_node = NULL;
        if (operator->codepoints[0] == (u32)'+')
        {
            operator_node = expr_create_geometric_sum_node();
        }
        else if (operator->codepoints[0] == (u32)'*')
        {
            operator_node = expr_create_geometric_product_node();
        }
        else
        {
            char* operator_str;
            utf8_codepoints_to_cstr(operator->codepoints, operator->length, &operator_str);
            operator_node = expr_create_function_node(operator_str);
            x_free(operator_str);
        }

        struct variadic_node* variadic_node = operator_node;

        struct json* operands = json->child->next; // json->child points to the operator (+ or *), json->child->next points to the array of children
        struct json* child = operands->child;
        void* child_node = expr_decode_json(child);
        expr_variadic_node_append_child(variadic_node, child_node);
        for (u64 i = 0; i < operands->length - 1; ++i)
        {
            child = child->next;
            void* next_child_node = expr_decode_json(child);
            expr_variadic_node_append_child(variadic_node, next_child_node);
        }

        return operator_node;
    }

    return NULL;
}

void* expr_decode_json_str(char const* json_str)
{
    u64 char_count = strlen(json_str) + 1;
    char* double_quoted_str = x_malloc(char_count);
    memcpy(double_quoted_str, json_str, char_count);
    for (u64 i = 0; i < char_count; ++i)
    {
        if (double_quoted_str[i] == '\'')
        {
            double_quoted_str[i] = '\"';
        }
    }

    u32* utf8_codepoints;
    u64 utf8_codepoints_len;
    utf8_cstr_to_codepoints(double_quoted_str, &utf8_codepoints, &utf8_codepoints_len);
    x_free(double_quoted_str);

    struct json_token* tokens;
    u64 tokens_len = json_tokenize_ot(utf8_codepoints, utf8_codepoints_len, &tokens);
    u64 token_index = 0;
    struct json* json = json_decode_tokens_ot(utf8_codepoints, tokens, &token_index, tokens_len);
    x_free(tokens);
    void* expr_node = expr_decode_json(json);
    json_free(json);
    x_free(utf8_codepoints);
    return expr_node;
}
