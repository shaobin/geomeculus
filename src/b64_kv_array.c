/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>

#include "b64_kv_array.h"
#include "mm.h"

struct b64_kv_array* b64_kv_array_create(u64 capacity)
{
    struct b64_kv_array* b64_kv_array = x_malloc(sizeof(struct b64_kv_array));
    b64_kv_array->length = 0;
    b64_kv_array->capacity = capacity;
    b64_kv_array->data = x_calloc(capacity, sizeof(struct b64_kv_pair));
    return b64_kv_array;
}

void b64_kv_array_free(struct b64_kv_array* b64_kv_array, u64 free_payload)
{
    if (free_payload)
    {
        for (u64 i = 0; i < b64_kv_array->length; ++i)
        {
            x_free(b64_kv_array->data[i].key.pointer);
            x_free(b64_kv_array->data[i].value.pointer);
        }
    }
    x_free(b64_kv_array->data);
    x_free(b64_kv_array);
}

void b64_kv_array_set(struct b64_kv_array* kv, union b64 key, union b64 value)
{
    for (u64 i = 0; i < kv->length; ++i)
    {
        if (kv->data[i].key.u64_value == key.u64_value)
        {
            kv->data[i].value.u64_value = value.u64_value;
            return;
        }
    }

    if (kv->length < kv->capacity)
    {
        kv->data[kv->length].key.u64_value = key.u64_value;
        kv->data[kv->length].value.u64_value = value.u64_value;
        kv->length++;
    }
    else
    {
        printf("b64_kv_array_set: b64_kv_array is full\n");
    }
}

u64 b64_kv_array_get(struct b64_kv_array* kv, union b64 key, union b64* value)
{
    for (u64 i = 0; i < kv->length; ++i)
    {
        if (kv->data[i].key.u64_value == key.u64_value)
        {
            value->u64_value = kv->data[i].value.u64_value;
            return 1;
        }
    }
    return 0;
}

/*
 * In the case of a pointer key, ownership of the key might be transferred to the b64_kv_array.
 * If ownership is transferred, ownership_transferred is set to 1, otherwise it is set to 0.
*/
void b64_kv_array_set_ex(struct b64_kv_array* kv, void* key, union b64 value, u64 (*compare)(void* key1, void* key2), u64* ownership_transferred)
{
    if (ownership_transferred != NULL)
    {
        *ownership_transferred = 0;
    }

    for (u64 i = 0; i < kv->length; ++i)
    {
        if (compare(kv->data[i].key.pointer, key))
        {
            kv->data[i].value.u64_value = value.u64_value;
            return;
        }
    }

    if (kv->length < kv->capacity)
    {
        kv->data[kv->length].key.pointer = key;
        if (ownership_transferred != NULL)
        {
            *ownership_transferred = 1;
        }
        kv->data[kv->length].value.u64_value = value.u64_value;
        kv->length++;
    }
    else
    {
        printf("b64_kv_array_set: b64_kv_array is full\n");
    }
}

u64 b64_kv_array_get_ex(struct b64_kv_array* kv, void* key, union b64* value, u64 (*compare)(void* key1, void* key2))
{
    for (u64 i = 0; i < kv->length; ++i)
    {
        if (compare(kv->data[i].key.pointer, key))
        {
            value->u64_value = kv->data[i].value.u64_value;
            return 1;
        }
    }
    return 0;
}

void b64_kv_array_iterate(struct b64_kv_array* kv, void (*callback)(union b64 key, union b64 value))
{
    for (u64 i = 0; i < kv->length; ++i)
    {
        callback(kv->data[i].key, kv->data[i].value);
    }
}

void b64_kv_array_print(struct b64_kv_array* kv)
{
    printf("b64_kv_array: length: %lu, capacity: %lu\n", kv->length, kv->capacity);
    for (u64 i = 0; i < kv->length; ++i)
    {
        printf("key: 0x%016lx, value: 0x%016lx\n", kv->data[i].key.u64_value, kv->data[i].value.u64_value);
    }
}
