/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <threads.h>
#include "numeric_types.h"

extern u64 f64_display_precision;
extern u64 f64_representation_precision;
extern u64 exponential_series_expansion_order;
extern u64 log_series_expansion_order;
extern u64 zeta_series_expansion_order;

struct algebra_context
{
    u64 dim_count;
    u64* dim_indexes;
    i32* dim_signatures;
};

extern thread_local struct algebra_context* algebra_ctx;

void algebra_ctx_free(struct algebra_context* ctx);
i32 algebra_ctx_dim_signature(u64 dim_index);
void algebra_ctx_create_from_signature(u64 num_positive_square, u64 num_negative_square, u64 num_zero_square, u64 starting_dim_index);
void algebra_ctx_print(struct algebra_context* ctx);

char* home_dir();
char* config_file_path();
char* history_file_path();
char* working_directory();

#endif
