/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include "id_signature.h"
#include "mm.h"

struct id_signature* id_signature_create()
{
    struct id_signature* id_signature = x_malloc(sizeof(struct id_signature));
    id_signature->length = 0;
    id_signature->data = NULL;
    return id_signature;
}

void id_signature_free(struct id_signature* id_signature)
{
    void* current = id_signature->data;
    while (current != NULL)
    {
        void* next = ((struct id_count*)current)->next;
        x_free(current);
        current = next;
    }

    x_free(id_signature);
}

/*
 * id_signature is an ordered linked list of id_count structs, ordered by id.
 */
void id_signature_set(struct id_signature* id_signature, i64 id)
{
    if((id != 0) && id_signature_unset(id_signature, -id))
    {
        return;
    }

    struct id_count* current = id_signature->data;
    struct id_count* previous = NULL;

    while (current != NULL)
    {
        if (current->id == id)
        {
            current->count++;
            return;
        }
        else if (current->id > id)
        {
            struct id_count* new_id_count = x_malloc(sizeof(struct id_count));
            new_id_count->id = id;
            new_id_count->count = 1;
            new_id_count->next = current;
            if (previous == NULL)
            {
                id_signature->data = new_id_count;
            }
            else
            {
                previous->next = new_id_count;
            }
            id_signature->length++;
            return;
        }
        previous = current;
        current = current->next;
    }

    struct id_count* new_id_count = x_malloc(sizeof(struct id_count));
    new_id_count->id = id;
    new_id_count->count = 1;
    new_id_count->next = NULL;
    if (previous == NULL)
    {
        id_signature->data = new_id_count;
    }
    else
    {
        previous->next = new_id_count;
    }
    id_signature->length++;
}

u64 id_signature_unset(struct id_signature* id_signature, i64 id)
{
    struct id_count* current = id_signature->data;
    struct id_count* previous = NULL;
    while (current != NULL)
    {
        if (current->id == id)
        {
            if (current->count == 1)
            {
                if (previous == NULL)
                {
                    id_signature->data = current->next;
                }
                else
                {
                    previous->next = current->next;
                }
                x_free(current);
                id_signature->length--;
            }
            else
            {
                current->count--;
            }
            return 1;
        }
        else if (current->id > id)
        {
            return 0;
        }
        previous = current;
        current = current->next;
    }
    return 0;
}

u64 id_signature_compare(void* signature_1, void* signature_2)
{
    struct id_signature* sig1 = signature_1;
    struct id_signature* sig2 = signature_2;

    if (sig1->length != sig2->length)
    {
        return 0;
    }

    struct id_count* current1 = sig1->data;
    struct id_count* current2 = sig2->data;
    while (current1 != NULL && current2 != NULL)
    {
        if (current1->id != current2->id)
        {
            return 0;
        }
        if (current1->count != current2->count)
        {
            return 0;
        }
        current1 = current1->next;
        current2 = current2->next;
    }
    if (current1 != NULL || current2 != NULL)
    {
        return 0;
    }
    return 1;

}

void id_signature_print(struct id_signature* id_signature)
{
    printf("id_signature (%lu elements) => ", id_signature->length);
    struct id_count* current = id_signature->data;
    while (current != NULL)
    {
        printf("(%ld, %lu) ", current->id, current->count);
        current = current->next;
    }
    printf("\n");
}

#ifdef MODULE_TEST
#include <stdio.h>
#include "cli_args.h"

static u64 force_run_all_tests = 0;

void test_id_signature_1(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct id_signature* id_signature = id_signature_create();
    id_signature_set(id_signature, -4);
    id_signature_set(id_signature, 1);
    id_signature_set(id_signature, 2);
    id_signature_set(id_signature, 2);
    id_signature_set(id_signature, 4);
    id_signature_set(id_signature, 3);
    id_signature_set(id_signature, 4);
    id_signature_set(id_signature, 4);
    id_signature_set(id_signature, 7);
    id_signature_set(id_signature, 9);
    id_signature_set(id_signature, 37);
    id_signature_set(id_signature, 7);

    id_signature_print(id_signature);

    id_signature_set(id_signature, -4);
    id_signature_unset(id_signature, 4);
    id_signature_set(id_signature, -4);

    id_signature_print(id_signature);

    id_signature_set(id_signature, 4);
    id_signature_set(id_signature, 4);

    id_signature_print(id_signature);

    id_signature_free(id_signature);
}

void test_id_signature_2(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct id_signature* id_signature = id_signature_create();
    id_signature_set(id_signature, 0);
    id_signature_set(id_signature, 1);
    id_signature_set(id_signature, 0);
    id_signature_set(id_signature, 0);

    id_signature_print(id_signature);

    id_signature_free(id_signature);

}

int main(int argc, char* argv[])
{
    mm_initialize();
    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_id_signature_1(1);
    test_id_signature_2(1);

    mm_finalize();
    mm_print_status();
    return 0;
}
#endif
