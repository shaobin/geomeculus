/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include "expr.h"
#include "mm.h"
#include "list.h"

/*
  Geometric sum and geometric product are associative, this function flattens out the terms in-place
*/
void* expr_flatten_node_inplace(void* node)
{
    if (!(expr_is_geometric_product_node(node) || expr_is_geometric_sum_node(node)))
    {
        return node;
    }

    struct variadic_node* var_node = node;
    struct expr_node* child = var_node->head;
    struct list* descendants = list_create();
    for (u64 i = 0; i < var_node->length; ++i)
    {
        expr_flatten_node_inplace(child);

        void* undocked_node = NULL;
        if(expr_cmp_variadic_types(child, var_node))
        {
            struct expr_node* grandchild = ((struct variadic_node*)child)->head;
            for (u64 j = 0; j < ((struct variadic_node*)child)->length; ++j)
            {
                list_append(descendants, grandchild);
                grandchild =grandchild->next;
            }
            undocked_node = child;
        }
        else
        {
            list_append(descendants, child);
        }

        child = child->next;

        if (undocked_node != NULL)
        {
            x_free(undocked_node);
        }
    }

    var_node->length = 0;
    var_node->head = NULL;
    var_node->tail = NULL;
    if (descendants->length > 0)
    {
        struct list_node* listnode = descendants->head;
        struct expr_node* childnode = listnode->data;
        expr_variadic_node_append_child(var_node, childnode);
        for (u64 i = 0; i < descendants->length - 1; ++i)
        {
            listnode = listnode->next;
            expr_variadic_node_append_child(var_node, listnode->data);
        }
    }

    list_free(descendants, 0);
    return node;
}

void* expr_remove_redundant_nodes(void* node)
{
    if (!(expr_is_geometric_product_node(node) || expr_is_geometric_sum_node(node)))
    {
        return expr_clone_tree(node);
    }

    struct variadic_node* var_node = node;

    if (var_node->length == 0)
    {
        return NULL;
    }

    if (var_node->length == 1)
    {
        struct expr_node* child = var_node->head;
        return expr_remove_redundant_nodes(child);
    }

    struct variadic_node* reduced_node = expr_create_variadic_node(var_node->type);

    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        void* reduced_child = expr_remove_redundant_nodes(child);
        if (reduced_child != NULL)
        {
            expr_variadic_node_append_child(reduced_node, reduced_child);
        }
        child = child->next;
    }

    if (reduced_node->length == 0)
    {
        x_free(reduced_node);
        return NULL;
    }

    if (reduced_node->length == 1)
    {
        struct expr_node* child = reduced_node->head;
        x_free(reduced_node);
        return child;
    }

    return reduced_node;
}
