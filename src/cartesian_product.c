/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "cartesian_product.h"
#include "mm.h"

/*
Python version:
result = [[]]
for list in lists:
    result = [x+[y] for x in result for y in list]
*/
void* cartesian_product(struct list* lists)
{
    struct list* result = list_create();
    list_append(result, list_create());

    struct list_node* input_list_node = lists->head;
    while (input_list_node != NULL) // iterate over input lists
    {
        struct list* input_list = (struct list*)input_list_node->data;
        struct list* new_result = list_create();

        struct list_node* result_node = result->head;
        while (result_node != NULL) // iterate over result lists
        {
            struct list* result_list = (struct list*)result_node->data;

            struct list_node* input_elem_node = input_list->head;
            while (input_elem_node != NULL) // iterate over each input list element
            {
                struct list* new_result_list = list_create();

                // add each element of the result list to the new result list
                struct list_node* result_list_node = result_list->head;
                while (result_list_node != NULL)
                {
                    list_append(new_result_list, result_list_node->data);
                    result_list_node = result_list_node->next;
                }

                list_append(new_result_list, input_elem_node->data);
                list_append(new_result, new_result_list);
                input_elem_node = input_elem_node->next;
            }

            result_node = result_node->next;
        }

        result_node = result->head;
        while (result_node != NULL)
        {
            struct list* result_list = (struct list*)result_node->data;
            list_clear(result_list, 0);
            x_free(result_list);
            result_node = result_node->next;
        }
        list_clear(result, 0);
        x_free(result);

        result = new_result;
        input_list_node = input_list_node->next;
    }

    return result;
}

#ifdef MODULE_TEST
#include <stdio.h>
#include "cli_args.h"

static u64 force_run_all_tests = 0;

void test_cartesian_product(u64 run)
{
    if (!force_run_all_tests && !run)
    {
        return;
    }

    struct list* list1 = list_create();
    list_append(list1, (void*)1);
    list_append(list1, (void*)2);
    list_append(list1, (void*)3);

    struct list* list2 = list_create();
    list_append(list2, (void*)4);
    list_append(list2, (void*)5);
    list_append(list2, (void*)6);

    struct list* list3 = list_create();
    list_append(list3, (void*)7);
    list_append(list3, (void*)8);

    struct list* lists = list_create();
    list_append(lists, list1);
    list_append(lists, list2);
    list_append(lists, list3);

    struct list* result = cartesian_product(lists);

    u64 tuple_count = 0;
    struct list_node* result_node = result->head;
    while (result_node != NULL)
    {
        struct list* result_list = (struct list*)result_node->data;
        struct list_node* result_list_node = result_list->head;
        while (result_list_node != NULL)
        {
            printf("%lu ", (u64)result_list_node->data);
            result_list_node = result_list_node->next;
        }
        tuple_count++;
        printf("\n");
        result_node = result_node->next;
    }

    printf("tuple_count: %lu\n", tuple_count);

    printf("\n");

    // free lists
    list_clear(list1, 0);
    x_free(list1);
    list_clear(list2, 0);
    x_free(list2);
    list_clear(list3, 0);
    x_free(list3);
    list_clear(lists, 0);
    x_free(lists);

    // free result
    result_node = result->head;
    while (result_node != NULL)
    {
        struct list* result_list = (struct list*)result_node->data;
        list_clear(result_list, 0);
        x_free(result_list);
        result_node = result_node->next;
    }
    list_clear(result, 0);
    x_free(result);
}

int main(int argc, char* argv[])
{
    mm_initialize();
    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    test_cartesian_product(1);

    mm_finalize();
    mm_print_status();
    return 0;
}

#endif
