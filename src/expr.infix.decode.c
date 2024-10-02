/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include <threads.h>

#include "expr.h"
#include "list.h"
#include "mm.h"
#include "func_repo.h"
#include "symbol.h"
#include "string_util.h"

/*
 * We use Pratt's algorithm (https://doi.org/10.1145/512927.512931) to parse infix expressions.
 * Each token has two associated functions, "nud" and "led", and integer representing binding power.
 *  nud: null denotation, called when a token appears at the beginning of a language construct
 *  led: left denotation, called when a token appears inside the construct
 *  lbp: left binding power
 *  rbp: right binding power
*/

static u64 const PRECEDENCE_SUM = 10;
static u64 const PRECEDENCE_PRODUCT = 20;
static u64 const PRECEDENCE_FUNCTION = 100;

static thread_local void* closed_expr = NULL;

static void* pratt_parse_tokens(struct list* tokens, u64* idx, u64 rbp);

static u64 pratt_get_precedence(char const* token)
{
    if (token == NULL)
    {
        return 0;
    }

    if (strcmp(token, "+") == 0)
    {
        return PRECEDENCE_SUM;
    }
    else if (strcmp(token, "*") == 0)
    {
        return PRECEDENCE_PRODUCT;
    }
    else if (strcmp(token, "(") == 0)
    {
        return PRECEDENCE_FUNCTION;
    }

    return 0;
}

static void* pratt_parse_token_with_null_denotation(struct list* tokens, u64* idx, char* token)
{
    if (token == NULL)
    {
        return NULL;
    }

    if (strcmp(token, "+") == 0)
    {
        return NULL;
    }
    else if (strcmp(token, "*") == 0)
    {
        return NULL;
    }
    else if (strcmp(token, "(") == 0) // '(' is a group operator when it has null denotation
    {
        void* expr = pratt_parse_tokens(tokens, idx, 0);
        if (strcmp(list_get(tokens, *idx), ")") != 0)
        {
            expr_free_tree(expr);
            printf("expected ')', but %s found\n", (char*)list_get(tokens, *idx));
            return NULL;
        }
        *idx += 1;
        closed_expr = expr;
        return expr;
    }
    else
    {
        void* leaf = expr_decode_leaf_node(token);
        if (leaf != NULL)
        {
            return leaf;
        }

        return expr_create_function_node(token);
    }
}

static u64 pratt_match_token(struct list* tokens, u64* idx, char const* token)
{
    char* token_at_idx = list_get(tokens, *idx);

    if (!token_at_idx)
    {
        return 0;
    }

    if (strcmp(token_at_idx, token) == 0)
    {
        *idx += 1;
        return 1;
    }
    return 0;
}

static void* pratt_parse_token_with_left_denotation(struct list* tokens, u64* idx, char const* token, void* left)
{
    if (token == NULL)
    {
        expr_free_tree(left);
        return NULL;
    }

    if (strcmp(token, "+") == 0)
    {
        u64 closed = left == closed_expr;

        void* right = pratt_parse_tokens(tokens, idx, PRECEDENCE_SUM);

        if (!closed && expr_is_geometric_sum_node(left))
        {
            expr_variadic_node_append_child(left, right);
            return left;
        }
        else
        {
            void* expr = expr_create_geometric_sum_node();
            expr_variadic_node_append_child(expr, left);
            expr_variadic_node_append_child(expr, right);
            return expr;
        }
    }
    else if (strcmp(token, "*") == 0)
    {
        u64 closed = left == closed_expr;

        void* right = pratt_parse_tokens(tokens, idx, PRECEDENCE_PRODUCT);

        if (!closed && expr_is_geometric_product_node(left))
        {
            expr_variadic_node_append_child(left, right);
            return left;
        }
        else
        {
            void* expr = expr_create_geometric_product_node();
            expr_variadic_node_append_child(expr, left);
            expr_variadic_node_append_child(expr, right);
            return expr;
        }
    }
    else if (strcmp(token, "(") == 0) // '(' is the function call
    {
        if (pratt_match_token(tokens, idx, ")"))
        {
            return left;
        }

        do
        {
            void* arg = pratt_parse_tokens(tokens, idx, 0);
            expr_variadic_node_append_child(left, arg);
        } while (pratt_match_token(tokens, idx, ","));

        if (pratt_match_token(tokens, idx, ")"))
        {
            return left;
        }
        else
        {
            expr_free_tree(left);
            printf("expected ')', but %s found\n", (char const*)list_get(tokens, *idx));
            return NULL;
        }
    }
    else if (strlen(token) > 0)
    {
        expr_free_tree(left);
        printf("unexpected token: %s\n", token);
        return NULL;
    }
    else
    {
        expr_free_tree(left);
        return NULL;
    }
}

static void* pratt_parse_tokens(struct list* tokens, u64* idx, u64 precedence)
{
    char* token = list_get(tokens, *idx);
    *idx += 1;

    void* left = pratt_parse_token_with_null_denotation(tokens, idx, token);
    while(precedence < pratt_get_precedence(list_get(tokens, *idx)))
    {
        token = list_get(tokens, *idx);
        *idx += 1;

        left = pratt_parse_token_with_left_denotation(tokens, idx, token, left);
    }

    return left;
}

void* expr_decode_infix_str(char const* expr_str)
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

                        char* named_expr_str = expr_encode_infix(expr, 0);
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
    void* expr = pratt_parse_tokens(tokens, &idx, 0);

    list_free(tokens, 1);
    return expr;
}
