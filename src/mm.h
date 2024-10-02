/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <stdio.h>
#include <threads.h>
#include "numeric_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern thread_local u64 mm_thread_id;

void mm_initialize();
void mm_finalize();
void mm_print_status();

void* x_malloc_impl(u64 size);
void* x_calloc_impl(u64 count, u64 size);
void x_free_impl(void* ptr);

#ifdef TRACE_MEMORY
void* x_malloc_trace(u64 size, char* file, u64 line);
void* x_calloc_trace(u64 count, u64 size, char* file, u64 line);
void x_free_trace(void* ptr, char* file, u64 line);
#define x_malloc(size) x_malloc_trace(size, __FILE__, __LINE__)
#define x_calloc(count, size) x_calloc_trace(count, size, __FILE__, __LINE__)
#define x_free(ptr) x_free_trace(ptr, __FILE__, __LINE__)
#else
#define x_malloc(size) x_malloc_impl(size)
#define x_calloc(count, size) x_calloc_impl(count, size)
#define x_free(ptr) x_free_impl(ptr)
#endif

#ifdef __cplusplus
}
#endif
#endif
