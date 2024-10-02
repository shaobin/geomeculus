/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ID_SIGNATURE_H
#define ID_SIGNATURE_H

#include "numeric_types.h"

struct id_count
{
    i64 id;
    u64 count;
    struct id_count* next;
};

struct id_signature
{
    u64 length;
    struct id_count* data;
};

struct id_signature* id_signature_create();
void id_signature_free(struct id_signature* id_signature);
void id_signature_set(struct id_signature* id_signature, i64 id);
u64 id_signature_unset(struct id_signature* id_signature, i64 id);
u64 id_signature_compare(void* sig1, void* sig2);
void id_signature_print(struct id_signature* id_signature);

#endif
