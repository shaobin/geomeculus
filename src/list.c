/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "list.h"
#include "mm.h"

struct list* list_create()
{
    struct list* list = (struct list*)x_malloc(sizeof(struct list));
    list->length = 0;
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_prepend(struct list* list, void* data)
{
    struct list_node* node = (struct list_node*)x_malloc(sizeof(struct list_node));
    node->data = data;
    node->next = list->head;
    node->prev = NULL;

    if (list->length == 0)
    {
        list->tail = node;
    }
    else
    {
        list->head->prev = node;
    }

    list->head = node;
    list->length++;
}

void list_append(struct list* list, void* data)
{
    struct list_node* node = x_malloc(sizeof(struct list_node));
    node->data = data;
    node->next = NULL;
    node->prev = list->tail;

    if (list->length == 0)
    {
        list->head = node;
    }
    else
    {
        list->tail->next = node;
    }

    list->tail = node;
    list->length++;
}

u64 list_contains(struct list* list, void* data)
{
    struct list_node* node = list->head;
    while (node != NULL)
    {
        if (node->data == data)
        {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

void* list_get(struct list* list, u64 index)
{
    if (index >= list->length)
    {
        return NULL;
    }
    struct list_node* node = list->head;
    for (u64 i = 0; i < index; i++)
    {
        node = node->next;
    }
    return node->data;
}

void list_iterate(struct list* list, void (*callback)(void* data))
{
    struct list_node* node = list->head;
    while (node != NULL)
    {
        callback(node->data);
        node = node->next;
    }
}

void list_remove(struct list* list, u64 index, u64 free_payload)
{
    if (index >= list->length)
    {
        return;
    }
    if (index == 0)
    {
        struct list_node* node = list->head;
        list->head = node->next;
        if (free_payload)
        {
            x_free(node->data);
        }
        x_free(node);
    }
    else
    {
        struct list_node* prev = list->head;
        for (u64 i = 0; i < index - 1; i++)
        {
            prev = prev->next;
        }
        struct list_node* node = prev->next;
        prev->next = node->next;
        if (free_payload)
        {
            x_free(node->data);
        }
        x_free(node);
    }
    list->length--;
}

void list_clear(struct list* list, u64 free_payload)
{
    while (list->length > 0)
    {
        list_remove(list, 0, free_payload);
    }
}

void list_free(struct list* list, u64 free_payload)
{
    list_clear(list, free_payload);
    x_free(list);
}

#ifdef MODULE_TEST
#include <stdio.h>
#include "cli_args.h"

static u64 force_run_all_tests = 0;

void print_u64(void* data)
{
    printf("%lu\n", (u64)data);
}

void test_list_prepend_append_remove(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct list* list = list_create();

    for (u64 i = 1; i <= 10; ++i)
    {
        if (i % 2 == 1)
        {
            list_prepend(list, (void*)i);
        }
        else
        {
            list_append(list, (void*)i);
        }
        list_iterate(list, print_u64);
        printf("\n");
    }

    list_remove(list, 0, 0);
    list_iterate(list, print_u64);
    printf("\n");

    list_remove(list, 5, 0);
    list_iterate(list, print_u64);
    printf("\n");

    list_remove(list, 7, 0);
    list_iterate(list, print_u64);
    printf("\n");

    list_free(list, 0);
}

int main(int argc, char* argv[])
{
    mm_initialize();
    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_list_prepend_append_remove(1);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
