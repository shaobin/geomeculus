/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "mm.h"
#include "spinlock.h"
#include "list.h"

u64 const SYMBOL_TYPE_UNDEFINED = 0;

// byte 1
u64 const SYMBOL_TYPE_NUMBER_MASK =    0x80;           // ... 1000 0000
u64 const SYMBOL_TYPE_NUMBER_INTEGER = 0x81;           // ... 1000 0001
u64 const SYMBOL_TYPE_NUMBER_REAL =    0x82;           // ... 1000 0010
u64 const SYMBOL_TYPE_NUMBER_COMPLEX = 0x88;           // ... 1000 1000
// byte 2
u64 const SYMBOL_TYPE_STRING =         0x8000;         // ... 1000 0000 0000 0000
// byte 3
u64 const SYMBOL_TYPE_FUNCTION =       0x800000;       // ... 1000 0000 0000 0000 0000 0000
// byte 4
u64 const SYMBOL_TYPE_VARIABLE =       0x80000000;     // ... 1000 0000 0000 0000 0000 0000 0000 0000

// byte 5
u64 const SYMBOL_TYPE_EXPR =           0x8000000000;   // ... 1000 0000 0000 0000 0000 0000 0000 0000 0000 0000

// byte 6
u64 const SYMBOL_TYPE_LIST =           0x800000000000; // ... 1000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000

/*
 * A registry is a collection of symbols.
 */
static struct list symbol_registry = {.length = 0, .head = NULL, .tail = NULL};
struct symbol symbol_undefined = {.type = SYMBOL_TYPE_UNDEFINED, .id = 0, .name = NULL};
static atomic_u64 symbol_id = 1;

u64 symbol_is_number(struct symbol* symbol)
{
    return symbol->type & SYMBOL_TYPE_NUMBER_MASK;
}

u64 symbol_is_real_number(struct symbol* symbol)
{
    return symbol->type == SYMBOL_TYPE_NUMBER_REAL;
}

u64 symbol_compare(struct symbol* a, struct symbol* b)
{
    if (a == b)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    if (a->type != b->type)
    {
        return 0;
    }

    return a->u64_value == b->u64_value;
}

struct symbol* symbol_find_by_id(u64 id)
{
    struct list_node* node = symbol_registry.head;
    while (node != NULL)
    {
        struct symbol* sym = node->data;
        if (sym->id == id)
        {
            return sym;
        }
        node = node->next;
    }
    return NULL;
}

struct symbol* symbol_find_by_name(char const* name)
{
    struct list_node* node = symbol_registry.head;
    while (node != NULL)
    {
        struct symbol* sym = node->data;
        if (sym->name != NULL && strcmp(sym->name, name) == 0)
        {
            return sym;
        }
        node = node->next;
    }
    return NULL;
}

struct symbol* symbol_create(u64 type, char const* name)
{
    struct symbol* symbol = x_malloc(sizeof(struct symbol));
    symbol->type = type;
    symbol->id = atomic_fetch_add(&symbol_id, 1);
    symbol->u64_value = 0;
    if (name == NULL)
    {
        symbol->name = NULL;
    }
    else
    {
        symbol->name = x_malloc(strlen(name) + 1);
        strcpy((char*)symbol->name, name);
    }
    list_append(&symbol_registry, symbol);

    return symbol;
}

void symbol_free(void* symbol, u64 free_payload)
{
    struct symbol* sym = symbol;

    if (sym->name != NULL)
    {
        x_free(sym->name);
    }

    if (free_payload)
    {
        x_free(sym->pointer);
    }

    x_free(symbol);
}

static void callback_free_symbol(void* symbol)
{
    symbol_free(symbol, 0);
}

static void callback_free_symbol_n_payload(void* symbol)
{
    symbol_free(symbol, 1);
}

void symbol_free_all(u64 free_payload)
{
    if (free_payload)
    {
        list_iterate(&symbol_registry, callback_free_symbol_n_payload);
    }
    else
    {
        list_iterate(&symbol_registry, callback_free_symbol);
    }
    list_clear(&symbol_registry, 0);
}

void symbol_print(void* sym)
{
    struct symbol* symbol = sym;
    if (symbol == NULL)
    {
        printf("NULL");
        return;
    }

    if (symbol->name != NULL)
    {
        printf("%s: ", symbol->name);
    }
    else if (symbol->type & SYMBOL_TYPE_NUMBER_MASK)
    {
        printf("number: ");
    }
    else if (symbol->type & SYMBOL_TYPE_STRING)
    {
        printf("string: ");
    }
    else if (symbol->type & SYMBOL_TYPE_FUNCTION)
    {
        printf("function: ");
    }
    else if (symbol->type & SYMBOL_TYPE_VARIABLE)
    {
        printf("variable: ");
    }
    else
    {
        printf("unamed_symbol: ");
    }

    if (symbol->type == SYMBOL_TYPE_FUNCTION)
    {
        printf("()");
    }
    else if (symbol_is_number(symbol))
    {
        if (symbol_is_real_number(symbol))
        {
            printf("%.6e", symbol->f64_value);
        }
        else
        {
            printf("%ld", symbol->i64_value);
        }
    }
    else if (symbol->type == SYMBOL_TYPE_STRING)
    {
        if (symbol->pointer == NULL)
        {
            printf("\"\"");
        }
        else
        {
            printf("\"%s\"", (char*)symbol->pointer);
        }
    }
    else
    {
        printf("unknown symbol type: %ld", symbol->type);
    }
    printf("\n");
}

void symbol_print_all()
{
    printf("\n");
    printf("----------------- Symbol Registry -----------------\n");
    list_iterate(&symbol_registry, symbol_print);
    printf("---------------------------------------------------\n");
    printf("\n");
}

#ifdef MODULE_TEST
#include <stdio.h>
int main(void)
{
    mm_initialize();

    f64 x = 2.718281828459045;
    struct symbol* sym_f64 = symbol_create(SYMBOL_TYPE_NUMBER_REAL, "e");
    sym_f64->f64_value = x;

    char* str = "hello world";
    struct symbol* sym_str = symbol_create(SYMBOL_TYPE_STRING, "hello");
    sym_str->pointer = str;

    i32 y = 5;
    struct symbol* sym_i32 = symbol_create(SYMBOL_TYPE_NUMBER_INTEGER, NULL);
    sym_i32->i64_value = y;

    symbol_print_all();
    symbol_free_all(0);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
