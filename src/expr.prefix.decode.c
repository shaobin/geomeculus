/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "list.h"
#include "func_repo.h"
#include "mm.h"
#include "string_util.h"

static void* decode_prefix_tokens(struct list* tokens, u64* idx)
{
    char* token = list_get(tokens, *idx);
    *idx += 1;
    if (*token != '(')
    {
        return expr_decode_leaf_node(token);
    }

    char* operator = list_get(tokens, *idx);
    *idx += 1;

    void* var_node = NULL;
    if(*operator == '+')
    {
        var_node = expr_create_geometric_sum_node();
    }
    else if (*operator == '*')
    {
        var_node = expr_create_geometric_product_node();
    }
    else
    {
        var_node = expr_create_function_node(operator);
    }

    while (1)
    {
        token = list_get(tokens, *idx);
        if (*token == ')')
        {
            *idx += 1;
            break;
        }

        void* child = decode_prefix_tokens(tokens, idx);
        expr_variadic_node_append_child(var_node, child);
    }

    return var_node;
}

/*
 * parse the S-expression like prefix expression into a tree of expr nodes
 * example: (v_1 + e_1) * (3.14 + e_2) -> (* (+ v_1 e_1) (+ 3.14 e_2))
 */
void* expr_decode_prefix_str(char const* expr_str)
{
    struct list* tokens = expr_tokenize_str(expr_str);
    if (tokens == NULL)
    {
        return NULL;
    }

    if (string_contains(expr_str, "$"))
    {
        struct list* named_expr_expanded_tokens = list_create();

        struct list_node* node_token = tokens->head;
        while (node_token)
        {
            char* token = node_token->data;

            if (*token == '$')
            {
                struct symbol* symbol = symbol_find_by_name(token + 1);
                if (symbol)
                {
                    void* expr = symbol->pointer;
                    if (expr)
                    {
                        x_free(node_token->data);

                        char* named_expr_str = expr_encode_prefix(expr, 0);
                        struct list* named_expr_tokens = expr_tokenize_str(named_expr_str);

                        struct list_node* named_expr_token = named_expr_tokens->head;
                        while (named_expr_token)
                        {
                            list_append(named_expr_expanded_tokens, named_expr_token->data);
                            named_expr_token = named_expr_token->next;
                        }

                        list_free(named_expr_tokens, 0);
                        x_free(named_expr_str);
                    }
                }
                else
                {
                    list_append(named_expr_expanded_tokens, token);
                }
            }
            else
            {
                list_append(named_expr_expanded_tokens, token);
            }

            node_token = node_token->next;
        }

        list_free(tokens, 0);
        tokens = named_expr_expanded_tokens;
    }

    u64 idx = 0;
    void* expr = decode_prefix_tokens(tokens, &idx);

    list_free(tokens, 1);
    return expr;
}
