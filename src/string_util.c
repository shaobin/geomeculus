/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <ctype.h>
#include <string.h>

#include "string_util.h"
#include "mm.h"
#include "list.h"

static u64 string_is_digit(char c)
{
    return c >= '0' && c <= '9';
}

u64 string_is_integer(char const* s)
{
    if (!s)
    {
        return 0;
    }

    u64 len = strlen(s);
    if (len == 0)
    {
        return 0;
    }

    u64 i = 0;
    if (s[0] == '-')
    {
        i = 1;
    }

    for (; i < len; ++i)
    {
        if (!string_is_digit(s[i]))
        {
            return 0;
        }
    }

    return 1;
}

char* string_trim_inplace(char* s)
{
    if (!s)
    {
        return NULL;
    }

    u64 len = strlen(s);
    u64 start = 0;
    u64 end = len - 1;
    while (start < len && isspace(s[start]))
    {
        ++start;
    }
    while (end > 0 && isspace(s[end]))
    {
        --end;
    }
    s[end + 1] = '\0';
    return s + start;
}

char* string_trim(char* s)
{
    if (!s)
    {
        return NULL;
    }

    if (*s == '\0')
    {
        return NULL;
    }

    u64 len = strlen(s);
    u64 start = 0;
    u64 end = len - 1;
    while (start < len && isspace(s[start]))
    {
        ++start;
    }
    while (end > 0 && isspace(s[end]))
    {
        --end;
    }
    char* result = x_malloc(end - start + 2);
    strncpy(result, s + start, end - start + 1);
    result[end - start + 1] = '\0';
    return result;
}

char* string_clone(char const* s)
{
    if (!s)
    {
        return NULL;
    }

    u64 len = strlen(s);
    char* result = x_malloc(len + 1);
    memcpy(result, s, len);
    return result;
}

u64 string_contains(char const* s, char const* sub)
{
    if (!s || !sub)
    {
        return 0;
    }

    u64 len = strlen(s);
    u64 sub_len = strlen(sub);
    if (sub_len > len)
    {
        return 0;
    }

    for (u64 i = 0; i < len - sub_len + 1; ++i)
    {
        u64 j = 0;
        for (j = 0; j < sub_len; ++j)
        {
            if (s[i + j] != sub[j])
            {
                break;
            }
        }

        if (j == sub_len)
        {
            return 1;
        }
    }

    return 0;
}

char* string_substring(char* s, u64 start_inclusive, u64 end_exclusive)
{
    if (!s)
    {
        return NULL;
    }

    if (start_inclusive >= end_exclusive)
    {
        return NULL;
    }

    u64 len = strlen(s);
    if (start_inclusive >= len)
    {
        return NULL;
    }

    if (end_exclusive > len)
    {
        return NULL;
    }

    char* result = x_malloc(end_exclusive - start_inclusive + 1);
    memcpy(result, s + start_inclusive, end_exclusive - start_inclusive);
    result[end_exclusive - start_inclusive] = '\0';
    return result;
}

struct list* string_split(char* s, char* delimiter, u64 trim)
{
    if (!s)
    {
        return NULL;
    }

    struct list* result = list_create();

    if (!delimiter)
    {
        if (trim)
        {
            char* trim_ = string_trim(s);
            if (trim_)
            {
                list_append(result, trim_);
            }
        }
        else
        {
            char* copy = x_malloc(strlen(s) + 1);
            strcpy(copy, s);
            list_append(result, copy);
        }

        return result;
    }

    size_t len = strlen(s);

    u64 start = 0;
    u64 end = 0;

    while (end < len)
    {
        if (s[end] == delimiter[0])
        {
            u64 i = 0;
            for (i = 0; i < strlen(delimiter); ++i)
            {
                if (end + i >= len || s[end + i] != delimiter[i])
                {
                    break;
                }
            }

            if (i == strlen(delimiter))
            {
                char* copy = x_malloc(end - start + 1);
                strncpy(copy, s + start, end - start);
                copy[end - start] = '\0';
                if (trim)
                {
                    char* trim_ = string_trim(copy);
                    if (trim_)
                    {
                        list_append(result, trim_);
                    }
                    x_free(copy);
                }
                else
                {
                    list_append(result, copy);
                }
                start = end + i;
                end = start;
            }
            else
            {
                ++end;
            }
        }
        else
        {
            ++end;
        }
    }

    if (start < len)
    {
        char* copy = x_malloc(end - start + 1);
        strncpy(copy, s + start, end - start);
        copy[end - start] = '\0';
        if (trim)
        {
            char* trim_ = string_trim(copy);
            if (trim_)
            {
                list_append(result, trim_);
            }
            x_free(copy);
        }
        else
        {
            list_append(result, copy);
        }
    }

    return result;
}

char* string_join(struct list* strings, char* delimiter)
{
    if (!strings)
    {
        return NULL;
    }

    if (!delimiter)
    {
        delimiter = "";
    }

    u64 len = 0;
    struct list_node* node = strings->head;
    while (node)
    {
        len += strlen((char*)node->data);
        node = node->next;
    }

    u64 delimiter_len = strlen(delimiter);

    len += (strings->length - 1) * delimiter_len;

    char* result = x_malloc(len + 1);
    result[len] = '\0';

    char* p = result;
    node = strings->head;
    while (node)
    {
        u64 size = strlen((char*)node->data);
        memcpy(p, node->data, size);
        p += size;

        node = node->next;

        if (delimiter_len > 0 && node)
        {
            memcpy(p, delimiter, delimiter_len);
            p += delimiter_len;
        }
    }

    return result;
}

char* string_replace(char* s, char* old, char* new)
{
    if (!s || !old || !new)
    {
        return NULL;
    }

    struct list* cols = string_split(s, old, 0);
    char* result = string_join(cols, new);
    list_free(cols, 1);
    return result;
}

#ifdef MODULE_TEST
#include <stdio.h>
#include "print_util.h"

void test_string_trim_inplace(void)
{
    char s[] = "  hello world  ";
    char* t = string_trim_inplace(s);
    printf("'%s'\n", t);
}

void test_string_trim(void)
{
    char* s = "  hello world  ";
    char* t = string_trim(s);
    printf("'%s'\n", t);
    x_free(t);
}

static void list_callback_print_string(void* s)
{
    printf("'%s'\n", (char*)s);
}

void test_string_split_1(void)
{
    print_block_line(__func__);
    char* s = "hello  world ";
    struct list* result = string_split(s, " ", 1);

    list_iterate(result, list_callback_print_string);

    list_free(result, 1);

    print_block_line("");
}

void test_string_split_2(void)
{
    print_block_line(__func__);
    char* s = "v2 + e1 + v1 ; v1=3, v2=2";
    struct list* result = string_split(s, ";", 1);

    list_iterate(result, list_callback_print_string);

    list_free(result, 1);
    print_block_line("");
}

void test_string_join(void)
{
    print_block_line(__func__);
    struct list* strings = list_create();
    list_append(strings, "hello");
    list_append(strings, "world");
    list_append(strings, "!");
    char* result = string_join(strings, " ");
    printf("'%s'\n", result);
    x_free(result);
    list_free(strings, 0);
    print_block_line("");
}

void test_string_replace(void)
{
    print_block_line(__func__);
    char* s = "hello world";
    char* result = string_replace(s, " ", "_");
    printf("'%s'\n", result);
    x_free(result);
    print_block_line("");
}

int main(void)
{
    mm_initialize();

    test_string_trim_inplace();
    test_string_trim();

    test_string_split_1();
    test_string_split_2();

    test_string_join();

    test_string_replace();

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
