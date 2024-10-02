/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include "numeric_types.h"

u64 string_is_integer(char const* s);
char* string_trim_inplace(char* s);
char* string_trim(char* s);
char* string_clone(char const* s);
u64 string_contains(char const* s, char const* sub);
char* string_substring(char* s, u64 start_inclusive, u64 end_exclusive);
struct list* string_split(char* s, char* delimiter, u64 trim);
char* string_join(struct list* strings, char* delimiter);
char* string_replace(char* s, char* old, char* new);

#endif
