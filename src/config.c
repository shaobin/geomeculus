/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "config.h"
#include "mm.h"

u64 f64_display_precision = 5;
u64 f64_representation_precision = 16;
u64 exponential_series_expansion_order = 20;
u64 log_series_expansion_order = 20;
u64 zeta_series_expansion_order = 20;

thread_local struct algebra_context* algebra_ctx = NULL;

void algebra_ctx_free(struct algebra_context* ctx)
{
    if (ctx)
    {
        x_free(ctx->dim_indexes);
        x_free(ctx->dim_signatures);
        x_free(ctx);
    }
}

void algebra_ctx_create_from_signature(u64 num_positive_square, u64 num_negative_square, u64 num_zero_square, u64 starting_dim_index)
{
    if (algebra_ctx)
    {
        algebra_ctx_free(algebra_ctx);
    }

    algebra_ctx = x_malloc(sizeof(struct algebra_context));
    algebra_ctx->dim_count = num_positive_square + num_negative_square + num_zero_square;
    algebra_ctx->dim_indexes = x_malloc(algebra_ctx->dim_count * sizeof(u64));
    algebra_ctx->dim_signatures = x_malloc(algebra_ctx->dim_count * sizeof(i32));

    for (u64 i = 0; i < num_positive_square; ++i)
    {
        algebra_ctx->dim_indexes[i] = i + starting_dim_index;
        algebra_ctx->dim_signatures[i] = 1;
    }

    for (u64 i = num_positive_square; i < (num_positive_square + num_negative_square); ++i)
    {
        algebra_ctx->dim_indexes[i] = i + starting_dim_index;
        algebra_ctx->dim_signatures[i] = -1;
    }

    for (u64 i = num_positive_square + num_negative_square; i < algebra_ctx->dim_count; ++i)
    {
        algebra_ctx->dim_indexes[i] = i + starting_dim_index;
        algebra_ctx->dim_signatures[i] = 0;
    }
}

i32 algebra_ctx_dim_signature(u64 dim_index)
{
    if (algebra_ctx)
    {
        for (u64 i = 0; i < algebra_ctx->dim_count; ++i)
        {
            if (algebra_ctx->dim_indexes[i] == dim_index)
            {
                return algebra_ctx->dim_signatures[i];
            }
        }
    }

    return 1;
}

void algebra_ctx_print(struct algebra_context* ctx)
{
    if (ctx)
    {
        printf("Explicitly specified dim signatures: %lu\n", ctx->dim_count);
        for (u64 i = 0; i < ctx->dim_count; ++i)
        {
            printf("dim %lu signature: %d\n",ctx->dim_indexes[i], ctx->dim_signatures[i]);
        }
    }
    else
    {
        printf("Not specified.\n");
    }
}

char* home_dir()
{
    struct passwd* pw = getpwuid(getuid());
    return pw->pw_dir;
}

char* config_file_path()
{
    static char path[PATH_MAX];
    if (snprintf(path, PATH_MAX, "%s/.geomeculus", home_dir()) >= PATH_MAX)
    {
        fprintf(stderr, "Error: config path too long\n");
        return NULL;
    }

    return path;
}

char* history_file_path()
{
    static char path[PATH_MAX];
    if (snprintf(path, PATH_MAX, "%s/.geomeculus_history", home_dir()) >= PATH_MAX)
    {
        fprintf(stderr, "Error: history path too long\n");
        return NULL;
    }

    return path;
}

char* working_directory()
{
    static char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) == NULL)
    {
        fprintf(stderr, "Error: getcwd failed\n");
        return NULL;
    }

    return path;
}
