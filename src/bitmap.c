/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include "bitmap.h"
#include "mm.h"

static u64 const BIT_GROUP_SIZE = 64;

struct bitmap* bitmap_create(u64 capacity)
{
    struct bitmap* bitmap = x_malloc(sizeof(struct bitmap));
    bitmap->bit_groups = (capacity + 63) >> 6;
    bitmap->data = x_calloc(bitmap->bit_groups, sizeof(u64));
    return bitmap;
}

void bitmap_free(struct bitmap* bitmap)
{
    x_free(bitmap->data);
    x_free(bitmap);
}

void bitmap_set(struct bitmap* bitmap, u64 index)
{
    u64 group = index >> 6;
    u64 bit = index & 0x3F; // 0x3F = 63
    bitmap->data[group] |= (U64_ONE << bit);
}

void bitmap_unset(struct bitmap* bitmap, u64 index)
{
    u64 group = index >> 6;
    u64 bit = index & 0x3F; // 0x3F = 63
    bitmap->data[group] &= ~(U64_ONE << bit);
}

u64 bitmap_get(struct bitmap* bitmap, u64 index)
{
    u64 group = index >> 6;
    u64 bit = index & 0x3F; // 0x3F = 63
    return (bitmap->data[group] >> bit) & U64_ONE;
}

u64 bitmap_compare(void* bm1, void* bm2)
{
    struct bitmap* bitmap1 = (struct bitmap*)bm1;
    struct bitmap* bitmap2 = (struct bitmap*)bm2;
    if (bitmap1->bit_groups != bitmap2->bit_groups)
    {
        return 0;
    }
    for (u64 group = 0; group < bitmap1->bit_groups; ++group)
    {
        if (bitmap1->data[group] != bitmap2->data[group])
        {
            return 0;
        }
    }
    return 1;
}

static u64 count_digit_ones(u64 value)
{
    u64 count = 0;
    while (value)
    {
        count += value & U64_ONE;
        value >>= U64_ONE;
    }
    return count;
}

u64 bitmap_count_set_bits(struct bitmap* bitmap)
{
    u64 count = 0;
    for (u64 group = 0; group < bitmap->bit_groups; ++group)
    {
        count += count_digit_ones(bitmap->data[group]);
    }
    return count;
}

u64 bitmap_first_set_bit_index(struct bitmap* bitmap)
{
    for (u64 group = 0; group < bitmap->bit_groups; ++group)
    {
        u64 data = bitmap->data[group];
        for (u64 bit = 0; bit < BIT_GROUP_SIZE; ++bit)
        {
            if ((data >> bit) & U64_ONE)
            {
                return group * BIT_GROUP_SIZE + bit;
            }
        }
    }
    return ~U64_ZERO;
}

/*
 * Iterate over all set bits in the bitmap and
 * call the callback function with the index of the bit.
*/
void bitmap_iterate(struct bitmap* bitmap, union b64 payload, void(*callback)(u64 index, union b64 payload))
{
    for (u64 group = 0; group < bitmap->bit_groups; ++group)
    {
        u64 data = bitmap->data[group];
        for (u64 bit = 0; bit < BIT_GROUP_SIZE; ++bit)
        {
            if ((data >> bit) & U64_ONE)
            {
                callback(group * BIT_GROUP_SIZE + bit, payload);
            }
        }
    }
}

void bitmap_print_bitmap(struct bitmap* bitmap)
{
    for (u64 group = 0; group < bitmap->bit_groups; ++group)
    {
        u64 data = bitmap->data[group];
        for (u64 bit = 0; bit < BIT_GROUP_SIZE; ++bit)
        {
            printf("%lu", (data >> bit) & U64_ONE);
        }
    }
    printf("\n");
}

#ifdef MODULE_TEST
#include "cli_args.h"

static u64 force_run_all_tests = 0;

void test_bitmap(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct bitmap* bitmap = bitmap_create(67);
    bitmap_set(bitmap, 0);
    bitmap_set(bitmap, 66);

    bitmap_print_bitmap(bitmap);

    bitmap_free(bitmap);
}

int main(int argc, char* argv[])
{
    mm_initialize();

    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_bitmap(1);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
