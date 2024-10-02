/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef U64_BITMAP_H_
#define U64_BITMAP_H_

#include "numeric_types.h"
#include "constants.h"

inline void u64_bitmap_set(u64* bitmap, u64 index)
{
    *bitmap |= (U64_ONE << index);
}

inline void u64_bitmap_unset(u64* bitmap, u64 index)
{
    *bitmap &= ~(U64_ONE << index);
}

inline u64 u64_bitmap_is_full(u64 bitmap)
{
    return bitmap == U64_MAX;
}

inline u64 u64_bitmap_first_unset_bit(u64 bitmap)
{
    for (u64 i = 0; i < 64; ++i)
    {
        if (((~bitmap) >> i) & U64_ONE)
        {
            return i;
        }
    }
    return 64; // 64 means no unset bit
}

#endif
