/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <string.h>

#include "expr.h"
#include "mm.h"
#include "utf8.h"
#include "list.h"
#include "func_repo.h"

u64 const EXPR_NODE_TYPE_LEAF =              0x8;                 // 1000
u64 const EXPR_NODE_TYPE_INSTANTIATED =      0x4;                 // 0100
u64 const EXPR_NODE_TYPE_BASIS_VECTOR =      0x2;                 // 0010

u64 const EXPR_NODE_TYPE_VARIADIC_MASK =     0xf0;                // 1111 0000
u64 const EXPR_NODE_TYPE_VARIADIC =          0x80;                // 1000 0000
u64 const EXPR_NODE_TYPE_GEOMETRIC_SUM =     0x10;                // 0001 0000
u64 const EXPR_NODE_TYPE_GEOMETRIC_PRODUCT = 0x20;                // 0010 0000
u64 const EXPR_NODE_TYPE_FUNCTION =          0x40;                // 0100 0000

u64 const EXPR_NODE_TYPE_MODIFIER_VARIABLE = 0x8000000000000000;  // 1 << 63
u64 const EXPR_NODE_TYPE_MODIFIER_SYMBOLIC = 0x0800000000000000;
u64 const EXPR_NODE_TYPE_MODIFIER_SCALAR   = 0x0080000000000000;

u64 const EXPR_INPUT_MODE_PREFIX = 1;
u64 const EXPR_INPUT_MODE_INFIX = 2;
u64 const EXPR_INPUT_MODE_JSON = 4;

u64 expr_is_variable_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_MODIFIER_VARIABLE;
}

u64 expr_is_symbolic_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_MODIFIER_SYMBOLIC;
}

u64 expr_is_leaf_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_LEAF;
}

u64 expr_is_instantiated_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_INSTANTIATED;
}

u64 expr_is_scalar_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_MODIFIER_SCALAR;
}

u64 expr_is_scalar_variable_node(void* node)
{
    return expr_is_scalar_node(node) && expr_is_variable_node(node);
}

u64 expr_is_nonvariable_scalar_node(void* node)
{
    return expr_is_scalar_node(node) && (!expr_is_variable_node(node));
}

u64 expr_is_function_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_FUNCTION;
}

u64 expr_is_scalar_function_node(void* node)
{
    return expr_is_function_node(node) && func_is_scalar(node);
}

u64 expr_is_instantiated_scalar_node(void* node)
{
    return expr_is_scalar_node(node) && expr_is_instantiated_node(node);
}

u64 expr_is_basis_vector_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_BASIS_VECTOR;
}

u64 expr_is_variadic_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_VARIADIC;
}

u64 expr_cmp_variadic_types(void* node1, void* node2)
{
    return (((struct expr_node*)node1)->type & EXPR_NODE_TYPE_VARIADIC_MASK)
        == (((struct expr_node*)node2)->type & EXPR_NODE_TYPE_VARIADIC_MASK);
}

u64 expr_is_geometric_product_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_GEOMETRIC_PRODUCT;
}

u64 expr_is_geometric_sum_node(void* node)
{
    return ((struct expr_node*)node)->type & EXPR_NODE_TYPE_GEOMETRIC_SUM;
}

struct leaf_node* expr_create_scalar_node_with_f64(f64 value)
{
    struct leaf_node* node = x_calloc(1, sizeof(struct leaf_node));
    node->symbol = &symbol_undefined;

    node->type = EXPR_NODE_TYPE_MODIFIER_SCALAR | EXPR_NODE_TYPE_LEAF | EXPR_NODE_TYPE_INSTANTIATED;
    node->f64_value = value;

    return node;
}

void expr_instantiate_scalar_node_with_f64(void* node, f64 value)
{
    struct leaf_node* leaf_node = node;

    leaf_node->f64_value = value;
    leaf_node->type |= EXPR_NODE_TYPE_INSTANTIATED;
    leaf_node->type &= ~EXPR_NODE_TYPE_MODIFIER_VARIABLE;
}

struct leaf_node* expr_create_scalar_variable_node(u64 var_id)
{
    struct leaf_node* node = x_calloc(1, sizeof(struct leaf_node));

    node->type = EXPR_NODE_TYPE_MODIFIER_SCALAR | EXPR_NODE_TYPE_LEAF | EXPR_NODE_TYPE_MODIFIER_VARIABLE;

    node->symbol = symbol_create(SYMBOL_TYPE_NUMBER_REAL, NULL);
    node->symbol->u64_value = var_id;

    return node;
}

struct leaf_node* expr_create_scalar_variable_node_with_symbol(struct symbol* sym, u64 var_id)
{
    struct leaf_node* node = x_calloc(1, sizeof(struct leaf_node));

    node->type = EXPR_NODE_TYPE_MODIFIER_SCALAR | EXPR_NODE_TYPE_LEAF | EXPR_NODE_TYPE_MODIFIER_VARIABLE;

    node->symbol = sym;
    node->symbol->u64_value = var_id;

    return node;
}

struct leaf_node* expr_create_basis_vector_node(u64 dim_index)
{
    struct leaf_node* node = x_calloc(1, sizeof(struct leaf_node));
    node->symbol = &symbol_undefined;
    node->type = EXPR_NODE_TYPE_BASIS_VECTOR | EXPR_NODE_TYPE_LEAF | EXPR_NODE_TYPE_INSTANTIATED;
    node->dim_index = dim_index;
    return node;
}

struct variadic_node* expr_create_variadic_node(u64 type)
{
    struct variadic_node* node = x_calloc(1, sizeof(struct variadic_node));
    node->symbol = &symbol_undefined;
    node->type = type | EXPR_NODE_TYPE_VARIADIC;
    return node;
}

struct variadic_node* expr_create_geometric_product_node()
{
    struct variadic_node* node = x_calloc(1, sizeof(struct variadic_node));
    node->symbol = &symbol_undefined;
    node->type = EXPR_NODE_TYPE_GEOMETRIC_PRODUCT | EXPR_NODE_TYPE_VARIADIC;
    return node;
}

struct variadic_node* expr_create_geometric_sum_node()
{
    struct variadic_node* node = x_calloc(1, sizeof(struct variadic_node));
    node->symbol = &symbol_undefined;
    node->type = EXPR_NODE_TYPE_GEOMETRIC_SUM | EXPR_NODE_TYPE_VARIADIC;
    return node;
}

/*
 * A function node is a variadic node takes one or more children as arguments and outputs an single expression.
 */
struct variadic_node* expr_create_function_node_with_symbol(struct symbol* sym)
{
    struct variadic_node* node = x_calloc(1, sizeof(struct variadic_node));
    node->symbol = sym;
    node->type = EXPR_NODE_TYPE_FUNCTION | EXPR_NODE_TYPE_VARIADIC;
    return node;
}

struct variadic_node* expr_create_function_node(char const* func_name)
{
    struct symbol* symbol = symbol_create(SYMBOL_TYPE_FUNCTION, func_name);
    symbol->u64_value = func_get_signature(func_name);

    return expr_create_function_node_with_symbol(symbol);
}

void* expr_func_argument(void* func)
{
    struct variadic_node* var_node = func;
    return var_node->head;
}

void dfs_callback_free_node(void* node)
{
    x_free(node);
}

void expr_free_tree(void* node)
{
    if (node != NULL)
    {
        expr_dfs(node, dfs_callback_free_node);
    }
}
