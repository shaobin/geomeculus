/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#ifndef CLI_ARGS_H
#define CLI_ARGS_H
#include "numeric_types.h"

u64 cli_check_flag(int argc, char* argv[], char const* key);
char* cli_get_arg_value_by_key(int argc, char* argv[], char const* key);

#endif
