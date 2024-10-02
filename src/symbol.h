/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SYMBOL_H
#define SYMBOL_H

#include "numeric_types.h"

extern u64 const SYMBOL_TYPE_UNDEFINED;

extern u64 const SYMBOL_TYPE_NUMBER;
extern u64 const SYMBOL_TYPE_NUMBER_INTEGER;
extern u64 const SYMBOL_TYPE_NUMBER_REAL;
extern u64 const SYMBOL_TYPE_NUMBER_COMPLEX;

extern u64 const SYMBOL_TYPE_STRING;
extern u64 const SYMBOL_TYPE_FUNCTION;
extern u64 const SYMBOL_TYPE_EXPR;
extern u64 const SYMBOL_TYPE_LIST;

struct symbol
{
    u64 type;
    u64 id;
    union {
        B64_MEMBERS;
    };
    char* name;
};

extern struct symbol symbol_undefined;

u64 symbol_is_number(struct symbol* symbol);
u64 symbol_is_real_number(struct symbol* symbol);
u64 symbol_compare(struct symbol* a, struct symbol* b);
struct symbol* symbol_find_by_id(u64 id);
struct symbol* symbol_find_by_name(char const* name);
struct symbol* symbol_create(u64 type, char const* name);
void symbol_free(void* symbol, u64 free_payload);
void symbol_free_all(u64 free_payload);
void symbol_print(void* sym);
void symbol_print_all();

#endif
