/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#ifndef SPECIAL_FUNC_H
#define SPECIAL_FUNC_H
#include "numeric_types.h"
#include "cuda_helper.h"

u64 factorial(u64 n);
f64 factorial_reciprocal(u64 n);
__device__ f64 boys(f64 n, f64 x);
__device__ i64 double_factorial(i64 n);

#endif
