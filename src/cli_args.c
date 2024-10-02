/*
 * This file includes code reused or adapted from LightAIMD by Bin Shao et al.
 * https://github.com/microsoft/LightAIMD
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 */
#include <stdlib.h>
#include <string.h>
#include "cli_args.h"

u64 cli_check_flag(int argc, char* argv[], char const* key)
{
    if (argc < 2)
    {
        return 0;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], key))
        {
            return 1;
        }
    }
    return 0;
}

char* cli_get_arg_value_by_key(int argc, char* argv[], char const* key)
{
    /* no command-line arguments are provided */
    if (argc < 2)
    {
        return NULL;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (!strcmp(argv[i], key))
        {
            if (i + 1 < argc)
            {
                return argv[i + 1];
            }
        }
    }
    return NULL;
}
