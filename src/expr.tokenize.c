/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "expr.h"
#include "list.h"
#include "mm.h"
#include "func_repo.h"
#include "string_util.h"

static char* create_expr_token(char const* str, u64 len)
{
    char* token = x_malloc(len + 1);
    memcpy(token, str, len);
    token[len] = '\0';
    return token;
}

static char separator_chars[] = {'(', ')', '+', '*', '{', '}', '[', ']', ',', ':', '"', '\''};

static u64 is_separator_char(char c)
{
    for (u64 i = 0; i < sizeof(separator_chars); ++i)
    {
        if (c == separator_chars[i])
        {
            return 1;
        }
    }
    return 0;
}

static u64 not_separator_char(char c)
{
    for (u64 i = 0; i < sizeof(separator_chars); ++i)
    {
        if (c == separator_chars[i])
        {
            return 0;
        }
    }
    return 1;
}

static inline u64 is_onenine(char c)
{
    return c >= '1' && c <= '9';
}

static inline u64 is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static inline u64 is_sign(char c)
{
    return c == '+' || c == '-';
}

static inline u64 is_exponent_symbol(char c)
{
    return c == 'e' || c == 'E';
}

static u64 scan_integer(char const* str, u64* idx)
{
    if (is_sign(str[*idx]))
    {
        ++(*idx);
    }

    if (str[*idx] == '0')
    {
        ++(*idx);
        return 1;
    }

    if (is_onenine(str[*idx]))
    {
        ++(*idx);
        while (is_digit(str[*idx]))
        {
            ++(*idx);
        }
        return 1;
    }

    return 0;
}

static u64 scan_fraction(char const* str, u64* idx)
{
    if (str[*idx] != '.')
    {
        return 0;
    }

    ++(*idx);
    if (is_digit(str[*idx]))
    {
        ++(*idx);
        while (is_digit(str[*idx]))
        {
            ++(*idx);
        }
        return 1;
    }

    return 0;
}

static u64 scan_exponent(char const* str, u64* idx)
{
    if (!is_exponent_symbol(str[*idx]))
    {
        return 0;
    }

    ++(*idx);
    if (is_sign(str[*idx]))
    {
        ++(*idx);
    }

    if (is_digit(str[*idx]))
    {
        ++(*idx);
        while (is_digit(str[*idx]))
        {
            ++(*idx);
        }
        return 1;
    }

    return 0;
}

static char* expr_extract_number_token(char const* expr_str, u64* idx)
{
    u64 start_idx = *idx;
    if (scan_integer(expr_str, idx) == 0)
    {
        return NULL;
    }

    scan_fraction(expr_str, idx);
    scan_exponent(expr_str, idx);

    return create_expr_token(expr_str + start_idx, *idx - start_idx);
}

struct list* expr_tokenize_str(char const* expr_str)
{
    if (expr_str == NULL)
    {
        return NULL;
    }

    u64 char_count = strlen(expr_str);

    if (char_count == 0)
    {
        return NULL;
    }

    struct list* tokens = list_create();
    u64 i = 0;
    while (i < char_count)
    {
        if (expr_str[i] == '-' || is_digit(expr_str[i]))
        {
            char* token = expr_extract_number_token(expr_str, &i);
            if (token == NULL)
            {
                printf("expr_tokenize_str: failed to extract number token\n");
                list_free(tokens, 1);
                return NULL;
            }
            list_append(tokens, token);
            continue;
        }

        if (is_separator_char(expr_str[i]))
        {
            list_append(tokens, create_expr_token(expr_str + i, 1));
            ++i;
            continue;
        }

        if (expr_str[i] == ' ')
        {
            ++i;
            continue;
        }

        u64 j = i + 1;
        while (j < char_count && not_separator_char(expr_str[j]) && expr_str[j] != ' ')
        {
            ++j;
        }
        list_append(tokens, create_expr_token(expr_str + i, j - i));
        i = j;
    }

    return tokens;
}

u64 expr_detect_input_mode(char const* expr_str)
{
    u64 default_mode = EXPR_INPUT_MODE_INFIX;
    if (expr_str == NULL)
    {
        return default_mode;
    }

    u64 char_count = strlen(expr_str);
    if (char_count == 0)
    {
        return default_mode;
    }

    struct list* tokens = list_create();
    u64 i = 0;
    while (i < char_count)
    {
        if (is_separator_char(expr_str[i]))
        {
            list_append(tokens, create_expr_token(expr_str + i, 1));
            if (tokens->length == 2)
            {
                break;
            }
            ++i;
            continue;
        }

        if (expr_str[i] == ' ')
        {
            ++i;
            continue;
        }

        u64 j = i + 1;
        while (j < char_count && not_separator_char(expr_str[j]) && expr_str[j] != ' ')
        {
            ++j;
        }
        list_append(tokens, create_expr_token(expr_str + i, j - i));
        if (tokens->length == 2)
        {
            break;
        }
        i = j;
    }

    if (tokens->length == 2)
    {
        char* token = list_get(tokens, 0);
        if (strcmp(token, "(") == 0)
        {
            token = list_get(tokens, 1);
            if (strcmp(token, "+") == 0 || strcmp(token, "*") == 0)
            {
                list_free(tokens, 1);
                return EXPR_INPUT_MODE_PREFIX;
            }

            void* leaf = expr_decode_leaf_node(token);
            if (leaf)
            {
                expr_free_tree(leaf);
                list_free(tokens, 1);
                return EXPR_INPUT_MODE_INFIX;
            }
            else if (isalpha(*token) || *token == '_')
            {
                list_free(tokens, 1);
                return EXPR_INPUT_MODE_PREFIX;
            }
            else
            {
                list_free(tokens, 1);
                return EXPR_INPUT_MODE_INFIX;
            }
        }
        else if (strcmp(token, "{") == 0)
        {
            list_free(tokens, 1);
            return EXPR_INPUT_MODE_JSON;
        }
    }

    list_free(tokens, 1);
    return default_mode;
}

void expr_print_tokens(struct list* tokens)
{
    if (tokens == NULL)
    {
        return;
    }

    printf("Tokens: ");
    for (u64 i = 0; i < tokens->length; ++i)
    {
        char* token = list_get(tokens, i);
        printf("%s ", token);
    }
    printf("\n");
}
