/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <string.h>

#include "expr.h"
#include "list.h"
#include "mm.h"

struct addr_mapping
{
    void* old_addr;
    void* new_addr;
};

struct expr_clone_data
{
    void* root;
    struct list* addr_mappings;
};

static void* get_new_addr(void* old_addr, struct list* addr_mappings)
{
    struct list_node* node = addr_mappings->head;
    while (node != NULL)
    {
        struct addr_mapping* mapping = node->data;
        if (mapping->old_addr == old_addr)
        {
            return mapping->new_addr;
        }
        node = node->next;
    }
    return NULL;
}

static void dfs_callback_clone_expr_node(void* dfs_node, void* data)
{
    void* node = ((struct top_down_dfs_node*)dfs_node)->node;
    u64 node_size = sizeof(struct expr_node);

    if (expr_is_scalar_node(node))
    {
        node_size = sizeof(struct leaf_node);
    }

    if (expr_is_basis_vector_node(node))
    {
        node_size = sizeof(struct leaf_node);
    }

    if (expr_is_variadic_node(node))
    {
        node_size = sizeof(struct variadic_node);
    }

    void* clone = x_malloc(node_size);
    memcpy(clone, node, node_size);

    if (expr_is_variadic_node(clone))
    {
        struct variadic_node* var_node = clone;
        var_node->length = 0;
        var_node->head = NULL;

        struct list* addr_mappings = ((struct expr_clone_data*)data)->addr_mappings;
        struct addr_mapping* mapping = x_malloc(sizeof(struct addr_mapping));
        mapping->old_addr = node;
        mapping->new_addr = clone;

        list_prepend(addr_mappings, mapping);
    }

    void* root = ((struct expr_clone_data*)data)->root;
    if (root == NULL)
    {
        ((struct expr_clone_data*)data)->root = clone;
        return;
    }

    struct variadic_node* parent_node = get_new_addr(((struct top_down_dfs_node*)dfs_node)->parent, ((struct expr_clone_data*)data)->addr_mappings);

    if (parent_node == NULL)
    {
        return;
    }

    if (parent_node->length == 0)
    {
        parent_node->head = clone;
        parent_node->length = 1;
        return;
    }

    struct expr_node* child = parent_node->head;
    for (u64 i = 0; i < parent_node->length - 1; ++i)
    {
        child = child->next;
    }
    child->next = clone;
    parent_node->length++;
}

void* expr_clone_tree(void* node)
{
    struct expr_clone_data data = {NULL, list_create()};
    expr_top_down_dfs(node, &data, dfs_callback_clone_expr_node);
    list_free(data.addr_mappings, 1);
    return data.root;
}
