/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#include "spinlock.h"

u32 threading_disabled = 1;

#ifdef MODULE_TEST
#include <stdio.h>

int main(void)
{
    volatile atomic_bool lck = 0;
    for (u64 i = 0; i < 10000; ++i)
    {
        spinlock_lock(&lck);
        spinlock_unlock(&lck);
    }
    return 0;
}
#endif
