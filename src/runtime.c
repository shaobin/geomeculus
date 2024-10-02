/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "mm.h"
#include "symbol.h"
#include "func_repo.h"

void runtime_initialize()
{
    mm_initialize();
    func_register_builtin_routines();
}

void runtime_finalize()
{
    func_free_repo();
    symbol_free_all(0);

    mm_finalize();
    mm_print_status();
}
