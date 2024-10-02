/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIST_H
#define LIST_H

#include "numeric_types.h"

struct list_node
{
    void* data;
    struct list_node *next;
    struct list_node *prev;
};

struct list
{
    u64 length;
    struct list_node *head;
    struct list_node *tail;
};

struct list* list_create();
void list_prepend(struct list* list, void* data);
void list_append(struct list* list, void* data);
u64 list_contains(struct list* list, void* data);
void* list_get(struct list* list, u64 index);
void list_iterate(struct list* list, void (*callback)(void* data));
void list_remove(struct list* list, u64 index, u64 free_payload);
void list_clear(struct list* list, u64 free_payload);
void list_free(struct list* list, u64 free_payload);

#endif
