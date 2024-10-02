/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef EXPR_H
#define EXPR_H

#include "numeric_types.h"
#include "symbol.h"
#include "json.h"
#include "b64_kv_array.h"
#include "list.h"

extern u64 const EXPR_INPUT_MODE_PREFIX;
extern u64 const EXPR_INPUT_MODE_INFIX;
extern u64 const EXPR_INPUT_MODE_JSON;

/*
 * type: expr node type
 * next: next sibling node
*/
#define EXPR_NODE_COMMON_MEMBERS \
    u64 type;                    \
    void* next;                  \
    void* prev;                  \
    struct symbol* symbol

struct expr_node
{
    EXPR_NODE_COMMON_MEMBERS;
};

struct leaf_node
{
    EXPR_NODE_COMMON_MEMBERS;
    union
    {
        union
        {
            B64_MEMBERS;
        };
        u64 dim_index;
    };
};

struct variadic_node
{
    EXPR_NODE_COMMON_MEMBERS;
    u64 length;
    void* head;
    void* tail;
};

struct top_down_dfs_node
{
    void* node;
    void* parent;
    u64 depth;
};

struct bottom_up_dfs_node
{
    void* node;
    void* parent;
    struct bottom_up_dfs_node* dfs_node_parent;
    u64 depth;
    u64 children_count;
    u64 children_processed;
};

// core
u64 expr_is_variable_node(void* node);
u64 expr_is_symbolic_node(void* node);
u64 expr_is_leaf_node(void* node);
u64 expr_is_instantiated_node(void* node);

u64 expr_is_scalar_node(void* node);
u64 expr_is_scalar_variable_node(void* node);
u64 expr_is_scalar_function_node(void* node);

u64 expr_is_nonvariable_scalar_node(void* node);
u64 expr_is_instantiated_scalar_node(void* node);
u64 expr_is_basis_vector_node(void* node);
u64 expr_is_variadic_node(void* node);
u64 expr_cmp_variadic_types(void* node1, void* node2);
u64 expr_is_geometric_product_node(void* node);
u64 expr_is_geometric_sum_node(void* node);
u64 expr_is_function_node(void* node);

struct leaf_node* expr_create_scalar_node_with_f64(f64 value);
void expr_instantiate_scalar_node_with_f64(void* node, f64 value);
struct leaf_node* expr_create_scalar_variable_node(u64 var_id);
struct leaf_node* expr_create_scalar_variable_node_with_symbol(struct symbol* sym, u64 var_id);
struct leaf_node* expr_create_basis_vector_node(u64 dim_index);

struct variadic_node* expr_create_variadic_node(u64 type);
struct variadic_node* expr_create_geometric_product_node();
struct variadic_node* expr_create_geometric_sum_node();
struct variadic_node* expr_create_function_node(char const* func_name);
struct variadic_node* expr_create_function_node_with_symbol(struct symbol* sym);
void* expr_func_argument(void* func);

void expr_free_tree(void* node);

// compare
u64 expr_compare(void* A, void* B);

// variadic
void expr_variadic_node_prepend_child(void* node, void* child);
void expr_variadic_node_append_child(void* node, void* child);
void expr_variadic_node_replace_child(void* node, void* child, void* new_child);
void expr_variadic_node_remove_child(void* node, void* child);

void* expr_flatten_node_inplace(void* node);
void* expr_remove_redundant_nodes(void* node);

void* expr_expand(void* node);
void* func_expand(void* func);
void* expr_reduce(void* node);
void* func_reduce(void* func);
void* expr_expand_reduce(void* expr);
void* func_expand_reduce(void* func);

void* expr_rotate_vector_in_plane(void* v, void* B, f64 theta);
void* func_rotate_vector_in_plane(void* func);

void expr_bfs(void* expr_root, void (*func)(void* data));
void expr_dfs(void* expr_root, void (*func)(void* node));
void expr_top_down_dfs(void* expr_root, void* data, void (*func)(void* dfs_node, void* data));
void expr_bottom_up_dfs(void* expr_root, void* data, void (*func)(void* dfs_node, void* data));

void* expr_clone_tree(void* node);

void* expr_assign_values_variables(void* expr, struct b64_kv_array* variable_values);
f64 expr_numerical_value(void* expr);
void* expr_evaluate_function(void* expr);
void* expr_evaluate(void* expr, struct b64_kv_array* variable_values);

// tokenize
struct list* expr_tokenize_str(char const* expr_str);
void expr_print_tokens(struct list* tokens);
u64 expr_detect_input_mode(char const* expr_str);

// encode
u64 expr_encode_leaf_node(void* node, char* buffer, u64* idx, u64 display_mode);
char* expr_encode_infix(void* node, u64 display_mode);
char* expr_encode_prefix(void* node, u64 display_mode);

// decode
void* expr_decode_leaf_node(char const* node_str);
void* expr_decode_infix_str(char const* infix_expr);
void* expr_decode_prefix_str(char const* prefix_str);
void* expr_decode_json(struct json* json);
void* expr_decode_json_str(char const* json_str);
char* expr_encode_to_json_str(void* expr);

// output / print
void expr_print_tree(void* node);
void expr_print_tree_with_label(void* node, char const* label);
void expr_print_infix(void* node);
void expr_print_infix_with_label(void* node, char* label);
void expr_print_prefix(void* node);
void expr_print_prefix_with_label(void* node, char* label);
void expr_print_json(void* node);
void expr_print_json_with_label(void* node, char* label);

void* expr_reverse(void* expr);
void* func_reverse(void* func);

// magnitude
void* expr_magnitude(void* expr);
void* func_magnitude(void* func);
void* expr_magnitude_squared(void* expr);
void* func_magnitude_squared(void* func);

void* expr_partial_derivative(void* expr, u64 var_idx);
void* func_partial_derivative(void* func_node);
void* expr_nth_partial_derivative(void* expr, u64 var_idx, u64 n);
void* func_nth_partial_derivative(void* func_node);
void* expr_gradient(void* expr, u64 n);
void* func_gradient(void* func_node);
void* expr_vector_derivative(void* F, void* M, u64 m);
void* func_vector_derivative(void* func_node);

u64 expr_grade_of_simplified_term(void* expr);
void* expr_k_vector_of_simplified_expr(void* expr, u64 k);
void* expr_grade(void* expr, u64 k);
void* func_grade(void* func);

// products
void* expr_geometric_product(void* A, void* B);
void* expr_scalar_product(void* A, void* B);
void* func_scalar_product(void* func);
void* expr_inner_product_of_simplied_terms(void* A, void* B);
void* expr_outer_product_of_simplied_terms(void* A, void* B);
void* expr_inner_product(struct list* exprs);
void* func_inner_product(void* func);
void* expr_outer_product(struct list* exprs);
void* func_outer_product(void* func);
void* expr_commutator_product(void* A, void* B);
void* func_commutator_product(void* func);
void* expr_anticommutator_product(void* A, void* B);
void* func_anticommutator_product(void* func);

// power
void* expr_power(void* expr, u64 n);
void* func_power(void* func);

void* expr_inverse_blade(void* A);
void* func_inverse_blade(void* func);

void* expr_project(void* b, void* A);
void* func_project(void* func);
void* expr_reject(void* b, void* A);
void* func_reject(void* func);

struct list* expr_reciprocal_basis(struct list* basis);
void expr_print_basis(struct list* basis);

// metrics
u64 expr_node_cardinality(void* node);
u64 expr_node_arithmetic_op_count(void* node);

// exponentiation
void* expr_exponential(void* expr);
void* func_exponential(void* func);

void* expr_exp_mv3(f64 a0, f64 a1, f64 a2, f64 a3, f64 a12, f64 a13, f64 a23, f64 a123);
void* expr_exp_3d(void* expr);
void* func_exp_3d(void* func_node);

#endif
