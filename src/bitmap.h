/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef BITMAP_H
#define BITMAP_H

#include "numeric_types.h"
#include "constants.h"

struct bitmap
{
    u64 bit_groups;
    u64* data;
};

struct bitmap* bitmap_create(u64 capacity);
void bitmap_free(struct bitmap* bitmap);
void bitmap_set(struct bitmap* bitmap, u64 index);
void bitmap_unset(struct bitmap* bitmap, u64 index);
u64 bitmap_get(struct bitmap* bitmap, u64 index);
u64 bitmap_compare(void* bm1, void* bm2);
u64 bitmap_count_set_bits(struct bitmap* bitmap);
u64 bitmap_first_set_bit_index(struct bitmap* bitmap);
void bitmap_iterate(struct bitmap* bitmap, union b64 data, void(*callback)(u64 index, union b64 data));
void bitmap_print_bitmap(struct bitmap* bitmap);

#endif
