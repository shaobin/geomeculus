/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>
#include "print_util.h"
#include "numeric_types.h"

void print_block_line(const char* name)
{
    u64 const width = 80;
    u64 const name_width = (name ? strlen(name) : 0) + 2;
    u64 const left_width = (width - name_width) / 2;
    u64 const right_width = width - name_width - left_width;

    printf("\n");
    for (u64 i = 0; i < left_width; ++i)
    {
        printf("-");
    }

    if (name)
    {
        printf(" %s ", name);
    }

    for (u64 i = 0; i < right_width; ++i)
    {
        printf("-");
    }
    printf("\n\n");
}
