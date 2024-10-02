/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <time.h>
#include "numeric_types.h"

extern u32 threading_disabled;

inline void __sleep__()
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1e6;
    nanosleep(&ts, NULL);
}

inline void spinlock_lock(volatile atomic_bool* lck)
{
    if (threading_disabled)
    {
        return;
    }

    while (1)
    {
        if (!atomic_exchange_explicit(lck, 1, memory_order_acquire))
        {
            return;
        }

        while (atomic_load_explicit(lck, memory_order_relaxed))
        {
            //__builtin_ia32_pause();
            __sleep__();
        }
    }
}

// try to lock one of the locks in the array with length len
inline volatile atomic_bool* spinlock_lock_one(volatile atomic_bool* lck, u64 len, u64* idx)
{
    if (threading_disabled)
    {
        return lck;
    }

    u64 start_idx = *idx;
    while(1)
    {
        for (u64 i = 0; i < len; ++i)
        {
            u64 index = (start_idx + i) % len;
            volatile atomic_bool* lck_i = lck + index;
            if (!atomic_exchange_explicit(lck_i, 1, memory_order_acquire))
            {
                *idx = index;
                return lck_i;
            }
        }
        __sleep__();
    }
}

inline void spinlock_unlock(volatile atomic_bool* lck)
{
    if (threading_disabled)
    {
        return;
    }

    atomic_store_explicit(lck, 0, memory_order_release);
}

#endif
