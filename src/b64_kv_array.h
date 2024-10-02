/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef B64_KV_ARRAY_H
#define B64_KV_ARRAY_H

#include "numeric_types.h"

struct b64_kv_pair {
    union b64 key;
    union b64 value;
};

struct b64_kv_array {
    u64 length;
    u64 capacity;
    struct b64_kv_pair* data;
};

struct b64_kv_array* b64_kv_array_create(u64 capacity);
void b64_kv_array_free(struct b64_kv_array* b64_kv_array, u64 free_payload);

void b64_kv_array_set(struct b64_kv_array* kv, union b64 key, union b64 value);
u64 b64_kv_array_get(struct b64_kv_array* kv, union b64 key, union b64* value);

void b64_kv_array_set_ex(struct b64_kv_array* kv, void* key, union b64 value, u64 (*compare)(void* key1, void* key2), u64* ownership_transferred);
u64 b64_kv_array_get_ex(struct b64_kv_array* kv, void* key, union b64* value, u64 (*compare)(void* key1, void* key2));

void b64_kv_array_iterate(struct b64_kv_array* kv, void (*callback)(union b64 key, union b64 value));
void b64_kv_array_print(struct b64_kv_array* kv);

#endif
