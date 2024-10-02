/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include <stdio.h>
#include "expr.h"
#include "func_repo.h"
#include "list.h"

static void dfs_callback_replace_variable_with_values(void* dfs_node, void* data)
{
    void* node = ((struct top_down_dfs_node*)dfs_node)->node;
    struct b64_kv_array* variable_values = (struct b64_kv_array*)data;

    if (expr_is_scalar_variable_node(node))
    {
        struct leaf_node* v_node = node;
        union b64 key;
        key.u64_value = v_node->symbol->u64_value;
        union b64 value;
        if (b64_kv_array_get(variable_values, key, &value))
        {
            expr_instantiate_scalar_node_with_f64(v_node, value.f64_value);
        }
        // else
        // {
        //     printf("dfs_callback_replace_variable_with_values: the value for variable %lu is not provided.\n", v_node->symbol->u64_value);
        // }
    }
}

void* expr_assign_values_variables(void* expr, struct b64_kv_array* variable_values)
{
    void* clone = expr_clone_tree(expr);
    expr_top_down_dfs(clone, variable_values, dfs_callback_replace_variable_with_values);
    return clone;
}

f64 expr_numerical_value(void* expr)
{
    void* simplified_expr = expr_expand_reduce(expr);

    if (expr_is_instantiated_node(simplified_expr))
    {
        struct leaf_node* scalar = simplified_expr;
        f64 result = scalar->f64_value;
        expr_free_tree(simplified_expr);
        return result;
    }
    else
    {
        expr_free_tree(simplified_expr);
        printf("expr_numerical_value: expression is not scalar\n");
        return NAN;
    }
}

static void dfs_callback_evaluate_func(void* node, void* data)
{
    struct top_down_dfs_node* dfs_node = node;
    struct variadic_node* func_node = dfs_node->node;
    if (expr_is_function_node(func_node))
    {
        struct list* children = list_create();
        struct expr_node* child = func_node->head;
        for (u64 i = 0; i < func_node->length; ++i)
        {
            void* undocked_child = NULL;
            if (expr_is_leaf_node(child))
            {
                list_append(children, child);
            }
            else
            {
                void* simplified = expr_expand_reduce(child);
                list_append(children, simplified);
                undocked_child = child;
            }
            child = child->next;

            if (undocked_child != NULL)
            {
                expr_free_tree(undocked_child);
            }
        }

        func_node->length = 0;
        func_node->head = NULL;
        func_node->tail = NULL;

        if (children->length > 0)
        {
            struct list_node* listnode = children->head;
            struct expr_node* childnode = listnode->data;
            expr_variadic_node_append_child(func_node, childnode);
            for (u64 i = 0; i < children->length - 1; ++i)
            {
                listnode = listnode->next;
                expr_variadic_node_append_child(func_node, listnode->data);
            }
        }

        list_free(children, 0);

        void* evaluated = func_evaluate(func_node);

        if (!evaluated)
        {
            evaluated = expr_create_function_node("null");
        }

        if (dfs_node->parent != NULL)
        {
            expr_variadic_node_replace_child(dfs_node->parent, func_node, evaluated);
        }
        else
        {
            *(void**)data = evaluated;
            expr_free_tree(func_node);
        }
    }
}

void* expr_evaluate_function(void* expr)
{
    void* clone = expr_clone_tree(expr);

    void* root = NULL;
    expr_bottom_up_dfs(clone, &root, dfs_callback_evaluate_func);

    if (root == NULL)
    {
        void* simp = expr_expand_reduce(clone);
        expr_free_tree(clone);
        return simp;
    }

    void* simp = expr_expand_reduce(root);
    expr_free_tree(root);
    return simp;
}

void* expr_evaluate(void* expr, struct b64_kv_array* variable_values)
{
    if (variable_values == NULL)
    {
        return expr_evaluate_function(expr);
    }

    void* assigned = expr_assign_values_variables(expr, variable_values);
    void* evaluated = expr_evaluate_function(assigned);
    expr_free_tree(assigned);
    return evaluated;
}
