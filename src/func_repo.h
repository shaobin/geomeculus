/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef FUNC_REPO_H
#define FUNC_REPO_H
#include "numeric_types.h"
#include "list.h"

typedef void* (*expr_func)(void*);

extern u64 const FUNC_FLAG_NONE;
extern u64 const FUNC_FLAG_SCALAR;

void func_register_builtin_routines();
void func_register(char const* name, expr_func func, expr_func derivative_func, u64 flags, char const* notes);
void func_print_repo();
u64 func_get_help(char const* name);
void func_get_names(struct list* func_names);
void func_free_repo();
char const* func_is_valid_name(char const* name);
u64 func_get_signature(char const* name);
u64 func_is_scalar(void* func);
void* func_evaluate(void* func);
void* func_derivative(void* func);

#endif
