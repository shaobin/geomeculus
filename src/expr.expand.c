/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "mm.h"
#include "list.h"
#include "cartesian_product.h"

static void* expr_expand_function_node(void* node)
{
    struct variadic_node* var_node = node;
    if (var_node->length == 0)
    {
        return expr_clone_tree(node);
    }

    struct variadic_node* expanded_node = expr_create_function_node_with_symbol(var_node->symbol);
    expanded_node->type = var_node->type;

    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        struct expr_node* expanded_child = expr_expand(child);
        expr_variadic_node_append_child(expanded_node, expanded_child);

        child = child->next;
    }

    return expanded_node;
}

/*
  Expand out the geometric addition into a sum of geometric products of basis vectors and constants
*/
static void* expr_expand_sum_node(void* sum_node)
{
    void* node = expr_clone_tree(sum_node);
    // step 1: flatten the node
    expr_flatten_node_inplace(node);

    // step 2: expand the children
    struct variadic_node* var_node = node;
    struct variadic_node* expanded_node = expr_create_geometric_sum_node();

    void* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        void* expanded_child = NULL;

        if (expr_is_variadic_node(child))
        {
            expanded_child = expr_expand(child);
        }
        else
        {
            expanded_child = expr_clone_tree(child);
        }

        // if the expanded term is a geometric sum, then we add its children to the expanded node
        if (expr_is_geometric_sum_node(expanded_child))
        {
            struct variadic_node* expanded_child_var_node = expanded_child;
            void* expanded_term_child = expanded_child_var_node->head;
            for (u64 j = 0; j < expanded_child_var_node->length; ++j)
            {
                void* _next = ((struct expr_node*)expanded_term_child)->next;
                expr_variadic_node_append_child(expanded_node, expanded_term_child); // expanded_term_child's next and prev pointers will be modified here
                expanded_term_child = _next;
            }
            x_free(expanded_child);
        }
        else
        {
            expr_variadic_node_append_child(expanded_node, expanded_child);
        }

        child = ((struct expr_node*)child)->next;
    }

    expr_free_tree(node);
    return expanded_node;
}

/*
Geometric product is distributive over geometric addition, this function expands out the products
*/
static void* expr_expand_product_node(void* product_node)
{
    void* node = expr_clone_tree(product_node);
    // step 1: flatten the node
    expr_flatten_node_inplace(node);

    // step 2: check whether the node is expandable
    struct variadic_node* var_node = node;
    void* child = var_node->head;

    u64 expandable = 0;

    for (u64 i = 0; i < var_node->length; ++i)
    {
        void* next_child = ((struct expr_node*)child)->next;
        if (expr_is_geometric_sum_node(child))
        {
            expr_variadic_node_replace_child(var_node, child, expr_expand_sum_node(child));
            expandable = 1;
        }
        else if (expr_is_geometric_product_node(child))
        {
            void* expanded_child = expr_expand_product_node(child);
            expr_variadic_node_replace_child(var_node, child, expanded_child);
            if (expr_is_geometric_sum_node(expanded_child))
            {
                expandable = 1;
            }
        }
        else if (expr_is_function_node(child))
        {
            expr_variadic_node_replace_child(var_node, child, expr_expand_function_node(child));
        }
        child = next_child;
    }

    if(!expandable)
    {
        return node;
    }

    // step 3: prepare the product terms to be expanded out
    struct list* terms = list_create();
    child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        if (expr_is_geometric_sum_node(child))
        {
            struct list* grandchildren = list_create();
            struct variadic_node* child_var_node = (struct variadic_node*)child;
            struct expr_node* grandchild = child_var_node->head;
            for (u64 j = 0; j < child_var_node->length; ++j)
            {
                list_append(grandchildren, grandchild);
                grandchild = grandchild->next;
            }
            list_append(terms, grandchildren);
        }
        else
        {
            struct list* grandchildren = list_create();
            list_append(grandchildren, child);
            list_append(terms, grandchildren);
        }
        child = ((struct expr_node*)child)->next;
    }

    // step 4: expand out the product terms
    struct list* expanded_terms = cartesian_product(terms);

    // free terms
    struct list_node* terms_listnode = terms->head;
    while(terms_listnode != NULL)
    {
        struct list* term = (struct list*)terms_listnode->data;
        list_clear(term, 0);
        x_free(term);
        terms_listnode = terms_listnode->next;
    }
    list_clear(terms, 0);
    x_free(terms);

    struct variadic_node* expanded_node = expr_create_geometric_sum_node();
    struct list_node* expanded_terms_listnode = expanded_terms->head;
    for (u64 i = 0; i < expanded_terms->length; ++i)
    {
        struct list* expanded_product_term = expanded_terms_listnode->data;

        struct variadic_node* expanded_product_term_node = expr_create_geometric_product_node();
        struct list_node* product_term_child = expanded_product_term->head;
        for (u64 j = 0; j < expanded_product_term->length; ++j)
        {
            struct expr_node* grandchild = expr_clone_tree(product_term_child->data);
            expr_variadic_node_append_child(expanded_product_term_node, grandchild);
            product_term_child = product_term_child->next;
        }
        expr_variadic_node_append_child(expanded_node, expanded_product_term_node);

        expanded_terms_listnode = expanded_terms_listnode->next;
    }

    // free expanded_terms
    expanded_terms_listnode = expanded_terms->head;
    while(expanded_terms_listnode != NULL)
    {
        struct list* expanded_product_term = (struct list*)expanded_terms_listnode->data;
        list_clear(expanded_product_term, 0);
        x_free(expanded_product_term);
        expanded_terms_listnode = expanded_terms_listnode->next;
    }
    list_clear(expanded_terms, 0);
    x_free(expanded_terms);

    expr_free_tree(node);

    return expanded_node;
}

/*
  An expanded expr can only take one of the following forms:
    I. a leaf node
    II. a product of leaf nodes
    III. a sum of multiple I & II
*/
void* expr_expand(void* node)
{
    if (expr_is_geometric_sum_node(node))
    {
        void* expanded = expr_expand_sum_node(node);
        void* flattened = expr_flatten_node_inplace(expanded);
        void* cleaned = expr_remove_redundant_nodes(flattened);
        expr_free_tree(flattened);
        return cleaned;
    }
    else if (expr_is_geometric_product_node(node))
    {
        void* expanded = expr_expand_product_node(node);
        void* flattened = expr_flatten_node_inplace(expanded);
        void* cleaned = expr_remove_redundant_nodes(flattened);
        expr_free_tree(flattened);
        return cleaned;

    }
    else if (expr_is_function_node(node))
    {
        return expr_expand_function_node(node);
    }
    else
    {
        return expr_clone_tree(node);
    }
}

void* func_expand(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_expand(var_node->head);
    }

    return expr_clone_tree(func);
}
