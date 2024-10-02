/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef MODULE_TEST
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>

#include "expr.h"
#include "mm.h"
#include "runtime.h"
#include "list.h"
#include "constants.h"
#include "cli_args.h"
#include "id_signature.h"
#include "print_util.h"
#include "func_repo.h"

static u64 force_run_all_tests = 0;
static u64 available_test_count = 0;
static u64 enabled_test_count = 0;
static u64 passed_test_count = 0;
static u64 failed_test_count = 0;

u64 test_filter(u64 run)
{
    available_test_count += 1;
    if (!force_run_all_tests && !run)
    {
        return 0;
    }

    enabled_test_count += 1;
    return 1;
}

void test_vec_a(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct leaf_node* vec_a = expr_create_basis_vector_node(0);

    struct leaf_node* vec_b = expr_create_basis_vector_node(1);

    struct leaf_node* c = expr_create_scalar_node_with_f64(3.14159265358979);

    struct variadic_node* gp = expr_create_geometric_product_node();
    expr_variadic_node_append_child(gp, c);
    expr_variadic_node_append_child(gp, vec_b);

    struct variadic_node* gs = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(gs, vec_a);
    expr_variadic_node_append_child(gs, gp);

    void* expected = expr_decode_infix_str("e0 + 3.1415926535897900 * e1");
    expr_print_infix_with_label(gs, "gs");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(gs, expected);
    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expected);
    expr_free_tree(gs);
}

void test_decode_json_expr_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "'e1'";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");
    void* expected = expr_decode_infix_str("e1");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_decode_json_expr_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "'3.1415899999999999e+00'";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");
    void* expected = expr_decode_infix_str("3.14159");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_decode_json_expr_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");
    void* expected = expr_decode_prefix_str("(+ (* e0 e1) (+ e2 e3 3.14159) (* e4 e5))");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_decode_json_expr_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['v2','e1']},{'+':['v2','e3','3.14159e+00']},{'*':['v1','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");
    void* expected = expr_decode_prefix_str("(+ (* v2 e1) (+ v2 e3 3.14159) (* v1 e5))");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_encode_json_expr_to_json_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "'e1'";

    printf("json_str:     %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    char* buffer = expr_encode_to_json_str(expr_node);
    printf("encoded json: %s\n", buffer);

    char* expected = "\"e1\"";

    u64 result = strcmp(buffer, expected) == 0;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    x_free(buffer);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_encode_json_expr_to_json_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "'3.14159'";

    printf("json_str:     %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    char* buffer = expr_encode_to_json_str(expr_node);
    printf("encoded json: %s\n", buffer);

    char* expected = "\"3.1415899999999999e+00\"";

    u64 result = strcmp(buffer, expected) == 0;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    x_free(buffer);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_encode_json_expr_to_json_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";

    printf("json_str:     %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    char* buffer = expr_encode_to_json_str(expr_node);
    printf("encoded json: %s\n", buffer);

    char* expected = "{\"+\": [{\"*\": [\"e0\", \"e1\"]}, {\"+\": [\"e2\", \"e3\", \"3.1415899999999999e+00\"]}, {\"*\": [\"e4\", \"e5\"]}]}";
    printf("expected:     %s\n", expected);

    u64 result = strcmp(buffer, expected) == 0;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    x_free(buffer);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_encode_json_expr_to_json_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['v2','e1']},{'+':['v2','e3','3.14159e+00']},{'*':['v1','e5']}]}";

    printf("json_str:     %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    char* buffer = expr_encode_to_json_str(expr_node);
    printf("encoded json: %s\n", buffer);

    char* expected = "{\"+\": [{\"*\": [\"v2\", \"e1\"]}, {\"+\": [\"v2\", \"e3\", \"3.1415899999999999e+00\"]}, {\"*\": [\"v1\", \"e5\"]}]}";
    printf("expected:     %s\n", expected);

    u64 result = strcmp(buffer, expected) == 0;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    x_free(buffer);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_expr_decode_prefix_str_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* prefix_str = "(* (+ (* e1 e2) (* e2 e3) (* e1 e3)) (* e3 e2 e1))";
    void* expr_node = expr_decode_prefix_str(prefix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("e3 + e1 + (-1.0 * e2)");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_prefix_str_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* prefix_str = "3.14E+00";
    void* expr_node = expr_decode_prefix_str(prefix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("3.14");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_prefix_str_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* prefix_str = "(+ 3.14E+00 2.718281828)";
    void* expr_node = expr_decode_prefix_str(prefix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_create_scalar_node_with_f64(3.14 + 2.718281828);

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_prefix_str_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* prefix_str = "(+ 3.14E+00 e1)";
    void* expr_node = expr_decode_prefix_str(prefix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(1));

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_prefix_str_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* prefix_str = "(sqrt (sin (+ v1 (sqrt (* 3.14E+00 e1)))))";
    void* expr = expr_decode_prefix_str(prefix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected
    void* prod = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(prod, expr_create_basis_vector_node(1));

    void* sqrt_prod = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt_prod, prod);

    void* sum = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(sum, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(sum, sqrt_prod);

    void* sin = expr_create_function_node("sin");
    expr_variadic_node_append_child(sin, sum);

    void* expected = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(expected, sin);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "3.14E+00 + 2.718281828";
    void* expr_node = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_create_scalar_node_with_f64(3.14 + 2.718281828);

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "3.14E+00 + e1";
    void* expr_node = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(1));

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "sqrt(sin((v1 + sqrt((3.14000 * e1)))))";
    void* expr = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected
    void* prod = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(prod, expr_create_basis_vector_node(1));

    void* sqrt_prod = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt_prod, prod);

    void* sum = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(sum, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(sum, sqrt_prod);

    void* sin = expr_create_function_node("sin");
    expr_variadic_node_append_child(sin, sum);

    void* expected = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(expected, sin);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "sqrt(sin((v1 + sqrt((3.14000 * e1))))) + e1 + e2 * v1";
    void* expr = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected
    void* prod = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(prod, expr_create_basis_vector_node(1));

    void* sqrt_prod = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt_prod, prod);

    void* sum = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(sum, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(sum, sqrt_prod);

    void* sin = expr_create_function_node("sin");
    expr_variadic_node_append_child(sin, sum);

    void* sqrt_sign = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt_sign, sin);

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, sqrt_sign);
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(1));

    void* prod2 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod2, expr_create_basis_vector_node(2));
    expr_variadic_node_append_child(prod2, expr_create_scalar_variable_node(1));

    expr_variadic_node_append_child(expected, prod2);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "e1 + e2 + v1 * (e3 + sqrt(3.14)) + v1";
    void* expr = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected
    void* sqrt = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt, expr_create_scalar_node_with_f64(3.14));

    void* sum = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(sum, expr_create_basis_vector_node(3));
    expr_variadic_node_append_child(sum, sqrt);

    void* prod = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(prod, sum);

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(2));
    expr_variadic_node_append_child(expected, prod);
    expr_variadic_node_append_child(expected, expr_create_scalar_variable_node(1));

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_6(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "e1 + (e2 + v1) + v1 * (e3 + sqrt(3.14 + 2.718E-03)) + v1";
    void* expr = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected

    void* pi_plus_e = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(pi_plus_e, expr_create_scalar_node_with_f64(3.14));
    expr_variadic_node_append_child(pi_plus_e, expr_create_scalar_node_with_f64(2.718E-03));

    void* sqrt = expr_create_function_node("sqrt");
    expr_variadic_node_append_child(sqrt, pi_plus_e);

    void* sum = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(sum, expr_create_basis_vector_node(3));
    expr_variadic_node_append_child(sum, sqrt);

    void* prod = expr_create_geometric_product_node();
    expr_variadic_node_append_child(prod, expr_create_scalar_variable_node(1));
    expr_variadic_node_append_child(prod, sum);

    void* e2_v1 = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(e2_v1, expr_create_basis_vector_node(2));
    expr_variadic_node_append_child(e2_v1, expr_create_scalar_variable_node(1));

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(expected, e2_v1);
    expr_variadic_node_append_child(expected, prod);
    expr_variadic_node_append_child(expected, expr_create_scalar_variable_node(1));

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_decode_infix_str_7(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_str = "((e1 + e2) + e3) + e4";
    void* expr = expr_decode_infix_str(infix_str);
    expr_print_infix_with_label(expr, "infix expr");

    // expected

    void* e1_plus_e2 = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(e1_plus_e2, expr_create_basis_vector_node(1));
    expr_variadic_node_append_child(e1_plus_e2, expr_create_basis_vector_node(2));

    void* e1_plus_e2_plus_e3 = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(e1_plus_e2_plus_e3, e1_plus_e2);
    expr_variadic_node_append_child(e1_plus_e2_plus_e3, expr_create_basis_vector_node(3));

    void* expected = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(expected, e1_plus_e2_plus_e3);
    expr_variadic_node_append_child(expected, expr_create_basis_vector_node(4));

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_reciprocal_scalar_function_node(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* node = expr_create_function_node("reciprocal");
    expr_variadic_node_append_child(node, expr_create_scalar_node_with_f64(2.718281828));
    expr_print_infix_with_label(node, "reciprocal node");

    void* evaluated = expr_evaluate_function(node);
    expr_print_infix_with_label(evaluated, "evaluated reciprocal node");

    void* expected = expr_create_scalar_node_with_f64(1.0 / 2.718281828);

    u64 result = expr_compare(evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(node);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
}

void test_flatten_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("(e0 * e1) + e2 + e3 + 3.14159 + (e4 * e5)");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':[{'*':['e0','e1']},{'*':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 * e1 * e2 * e3 * 3.14159 * e4 * e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'+':['e4','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 * e1 + e2 + e3 + 3.14159 + e4 + e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'+':['e0','e1']},{'+':['e2','e3','3.14e+00']},{'+':['e4','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 + e1 + e2 + e3 + 3.14000 + e4 + e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'+':['e0','e1']},{'+':['e2','e3','3.14e+00']},{'+':[{'+':['e4',{'+':['e0','e1']}]},'e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 + e1 + e2 + e3 + 3.14000 + e4 + e0 + e1 + e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_6(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':[{'*':['e0','e1']},{'*':['e2','e3','3.14e+00']},{'*':[{'+':['e4',{'*':['e0','e1']}]},'e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 * e1 * e2 * e3 * 3.14000 * (e4 + e0 * e1) * e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_7(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':[{'*':['e0','e1']},{'*':['e2','e3','3.14e+00']},{'*':[{'+':['e4',{'+':['e0','e1']}]},'e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("e0 * e1 * e2 * e3 * 3.14000 * (e4 + e0 + e1) * e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_flatten_8(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['v2','e1']},{'+':['v2','e3','3.14159e+00']},{'*':['v1','e5']}]}";
    void* expr = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr, "expr");

    expr_flatten_node_inplace(expr);
    expr_print_infix_with_label(expr, "flattened expr_clone");

    void* expected = expr_decode_infix_str("v2 * e1 + v2 + e3 + 3.14159 + v1 * e5");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_expr_variadic_node_append_child(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    char* json_str_2 = "{'+':['e6','e7']}";
    printf("json_str_2: %s\n", json_str_2);
    void* expr_node_2 = expr_decode_json_str(json_str_2);
    expr_print_infix_with_label(expr_node_2, "infix expr_2");

    expr_variadic_node_append_child(expr_node, expr_node_2);
    expr_print_infix_with_label(expr_node, "result expr");

    void* expected = expr_decode_infix_str("e0 * e1 + (e2 + e3 + 3.14159) + e4 * e5 + (e6 + e7)");
    expr_print_tree_with_label(expected, "expected");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr_node, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expected);
}

void test_expr_variadic_node_replace_child(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    char* json_str_2 = "{'+':['e6','e7']}";
    printf("json_str_2: %s\n", json_str_2);
    void* expr_node_2 = expr_decode_json_str(json_str_2);
    expr_print_infix_with_label(expr_node_2, "infix expr_2");

    expr_variadic_node_replace_child(expr_node, ((struct variadic_node*)expr_node)->head, expr_node_2);
    expr_print_infix_with_label(expr_node, "result expr");

    void* expected = expr_decode_infix_str("(e6 + e7) + (e2 + e3 + 3.14159) + e4 * e5");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr_node, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expected);
}

void test_expr_variadic_node_remove_child(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3','3.14159e+00']},{'*':['e4','e5']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    struct expr_node* child = ((struct variadic_node*)expr_node)->head;
    child = child->next;

    expr_variadic_node_remove_child(expr_node, child);
    expr_print_infix_with_label(expr_node, "result expr");

    void* expected = expr_decode_infix_str("e0 * e1 + e4 * e5");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr_node, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expected);
}

void test_expand_expr_product_node_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':[{'+':['e0','e1']},{'+':['e2','e3']},{'+':['e4','e5']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("e0 * e2 * e4 + e0 * e2 * e5 + e0 * e3 * e4 + e0 * e3 * e5 + e1 * e2 * e4 + e1 * e2 * e5 + e1 * e3 * e4 + e1 * e3 * e5");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expand_expr_product_node_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':[{'+':['v0','e1']},{'+':['e2','v0']},{'+':['e4','v1']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("v0 * e2 * e4 + v0 * e2 * v1 + v0 * v0 * e4 + v0 * v0 * v1 + e1 * e2 * e4 + e1 * e2 * v1 + e1 * v0 * e4 + e1 * v0 * v1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;

    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expand_expr_sum_node_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3']},{'*':[{'+':['e0','e1']},{'+':['e4','e5']}]}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("(e0 * e1) + e2 + e3 + (e0 * e4) + (e0 * e5) + (e1 * e4) + (e1 * e5)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expand_expr_sum_node_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3']},{'*':[{'+':[{'*':['e0','e1']},{'*':['e2','e3']}]},{'+':['e4','e5']}]}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("(e0 * e1) + e2 + e3 + (e0 * e1 * e4) + (e0 * e1 * e5) + (e2 * e3 * e4) + (e2 * e3 * e5)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expand_expr_sum_node_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['e2','e3']}, {'+':['e8']}, {'*':['e8']}, {'*':[{'+':[{'*':['e0','e1']},{'*':['e2','e3']}]},{'+':['e4','e5']}]}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("(e0 * e1) + e2 + e3 + e8 + e8 + (e0 * e1 * e4) + (e0 * e1 * e5) + (e2 * e3 * e4) + (e2 * e3 * e5)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;

    }

    expr_free_tree(expected);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_expand_expr_sum_node_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':[{'*':['e0','e1']},{'+':['v2','e3']}, {'+':['v8']}, {'*':['e8']}, {'*':[{'+':[{'*':['e0','e1']},{'*':['v2','e3']}]},{'+':['v3','e5']}]}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* expected = expr_decode_infix_str("(e0 * e1) + v2 + e3 + v8 + e8 + (e0 * e1 * v3) + (e0 * e1 * e5) + (v2 * e3 * v3) + (v2 * e3 * e5)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expanded_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expected);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    printf("\n");
}

void test_expr_reduce_product_node_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['e0','e1','e0','e5']}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* reduced_expr = expr_reduce(expr_node);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("-1.0 * e1 * e5");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_product_node_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['e0','e1','e0','e5', 'e7', {'+':['e0','e1']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("e0 * e1 * e5 * e7 + -1.00000 * e5 * e7");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_product_node_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['v0','e1','v0','e5', 'e7', {'+':['v0','e1']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("v0 * v0 * v0 * e1 * e5 * e7 + v0 * v0 * e5 * e7");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(expanded_expr);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_sum_node_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':['e0','e1','e0','e5']}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* reduced_expr = expr_reduce(expr_node);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("2.0 * e0 + e1 + e5");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_sum_node_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':['e0',{'*':['e0','e1','e0','e5']},'e0','e5']}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* reduced_expr = expr_reduce(expr_node);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("2.00000 * e0 + -1.00000 * e1 * e5 + e5");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_sum_node_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':['e0',{'*':['e0','e1','e0','e5']},'e0', {'*':['e0','e1','e0','e5', 'e7']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* reduced_expr = expr_reduce(expr_node);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("2.00000 * e0 + -1.00000 * e1 * e5 + -1.00000 * e1 * e5 * e7");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_reduce_sum_node_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr_node = expr_decode_infix_str("0 + e1");
    expr_print_infix_with_label(expr_node, "infix expr");

    void* reduced_expr = expr_reduce(expr_node);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("e1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    expr_free_tree(reduced_expr);
    expr_free_tree(expected);

    printf("\n");
}

void test_expand_reduce_expr_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'+':['e0',{'*':['e0','e1','e0','e5']},'e0', {'*':['e0','e1','e0','e5', 'e7', {'+':['e0','e1']}]}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("2.00000 * e0 + -1.00000 * e1 * e5 + e0 * e1 * e5 * e7 + -1.00000 * e5 * e7");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);
    printf("\n");
}

void test_expand_reduce_expr_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['v0','e1','v0','e5', 'e7', {'+':['v0','e1']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("v0 * v0 * v0 * e1 * e5 * e7 + v0 * v0 * e5 * e7");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);
    printf("\n");
}

void test_expand_reduce_expr_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['v0','e1','v0','e5', 'e7', '3.0', {'+':['v0','e1', 'e0']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("3.00000 * v0 * v0 * v0 * e1 * e5 * e7 + (3.00000 * v0 * v0 * e5 * e7) + (-3.00000 * v0 * v0 * e0 * e1 * e5 * e7)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);
    printf("\n");
}

int test_expand_reduce_expr_from_file(char* program_path, u64 run)
{
    if(!test_filter(run))
    {
        return 0;
    }

    print_block_line(__func__);

    char buffer[PATH_MAX];
    char parent_dir_buffer[PATH_MAX];
    char json_file[PATH_MAX + 128];
    char json_file_abspath[PATH_MAX];

    if (realpath(program_path, buffer) == NULL)
    {
        perror("Error finding absolute file path");
        return 1;
    }

    // Get the parent directory path
    char* parentDir = dirname(buffer);
    if (realpath(parentDir, parent_dir_buffer) == NULL)
    {
        perror("Error finding parent directory path");
        return 1;
    }

    sprintf(json_file, "%s/../../../samples/sample-expr.json", parent_dir_buffer);

    if (realpath(json_file, json_file_abspath) == NULL)
    {
        perror("Error finding absolute file path for basis set directory");
        return 1;
    }

    printf("reading json_file: %s\n", json_file_abspath);

    u32* codepoints;
    u64 codepoints_len;
    json_read_into_codepoints_ot(json_file_abspath, &codepoints, &codepoints_len);
    struct json_token* tokens;
    u64 tokens_len = json_tokenize_ot(codepoints, codepoints_len, &tokens);
    u64 token_index = 0;
    struct json* json = json_decode_tokens_ot(codepoints, tokens, &token_index, tokens_len);
    void* expr_node = expr_decode_json(json);

    json_free(json);
    x_free(tokens);
    x_free(codepoints);

    expr_print_infix_with_label(expr_node, "infix expr");
    printf("\n");

    void* expanded_expr = expr_expand(expr_node);
    expr_print_infix_with_label(expanded_expr, "expanded expr");
    printf("\n");

    void* reduced_expr = expr_reduce(expanded_expr);
    expr_print_infix_with_label(reduced_expr, "reduced expr");

    void* expected = expr_decode_infix_str("348 + 348 * e0 + -180 * e0 * e1 + 180 * e1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(reduced_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(reduced_expr);
    expr_free_tree(expanded_expr);
    expr_free_tree(expr_node);
    expr_free_tree(expected);
    printf("\n");
    return 0;
}

void test_expr_rotate_vector_in_plane_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* v = expr_decode_json_str(
        "{'+':\
            [\
                {'*':['3','e0']},\
                {'*':['2','e1']},\
                'e2'\
            ]\
        }"
    );
    expr_print_infix_with_label(v, "v");
    printf("\n");

    void* B = expr_decode_json_str(
        "{'*':\
            [\
                'e0',\
                'e1'\
            ]\
        }"
    );
    expr_print_infix_with_label(B, "B");
    printf("\n");

    void* rotated_v = expr_rotate_vector_in_plane(v, B, M_PI * 0.5);
    expr_print_infix_with_label(rotated_v, "rotated v");

    void* expected  = expr_decode_infix_str("-2 * e0 + 3 * e1 + e2");

    u64 result = expr_compare(rotated_v, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(rotated_v);
    expr_free_tree(v);
    expr_free_tree(B);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_rotate_vector_in_plane_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    void* v = expr_decode_json_str(
        "{'+':\
            [\
                'e0',\
                'e2'\
            ]\
        }"
    );
    expr_print_infix_with_label(v, "v");
    printf("\n");

    void* a = expr_decode_json_str(
        "{'+':\
            [\
                'e0',\
                'e2'\
            ]\
        }"
    );

    void* b = expr_decode_json_str(
        "{'+':\
            [\
                'e1',\
                'e2'\
            ]\
        }"
    );

    struct list* a_b = list_create();
    list_append(a_b, a);
    list_append(a_b, b);

    void* a_wedge_b = expr_outer_product(a_b);
    list_free(a_b, 0);

    void* B = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(B, a_wedge_b);
    // 0.5 / math.sin(math.pi / 3) == 0.5773502691896258
    expr_variadic_node_prepend_child(B, expr_create_scalar_node_with_f64(0.5773502691896258));

    expr_print_infix_with_label(B, "B");
    printf("\n");

    void* rotated_v = expr_rotate_vector_in_plane(v, B, M_PI / 3);
    expr_print_infix_with_label(rotated_v, "rotated v");

    void* simp_rotated_v = expr_expand_reduce(rotated_v);
    expr_print_infix_with_label(simp_rotated_v, "simplified rotated v");

    void* expected  = expr_decode_infix_str("e2 + e1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simp_rotated_v, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(rotated_v);
    expr_free_tree(simp_rotated_v);
    expr_free_tree(v);
    expr_free_tree(a);
    expr_free_tree(b);
    expr_free_tree(B);
    expr_free_tree(expected);

    printf("\n");
}

void test_expr_rotate_vector_in_plane_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* v = expr_decode_json_str(
        "{'+':\
            [\
                {'*': ['0.5', 'e1110']},\
                {'*': ['0.5', 'e1111']},\
                'e1112'\
            ]\
        }"
    );
    expr_print_infix_with_label(v, "v");
    printf("\n");

    void* a = expr_decode_json_str(
        "{'+':\
            [\
                'e1110',\
                'e1112'\
            ]\
        }"
    );

    void* b = expr_decode_json_str(
        "{'+':\
            [\
                'e1111',\
                'e1112'\
            ]\
        }"
    );

    struct list* a_b = list_create();
    list_append(a_b, a);
    list_append(a_b, b);

    void* a_wedge_b = expr_outer_product(a_b);
    list_free(a_b, 0);

    void* B = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(B, a_wedge_b);
    // 0.5 / math.sin(math.pi / 3) == 0.5773502691896258
    expr_variadic_node_prepend_child(B, expr_create_scalar_node_with_f64(0.5773502691896258));

    expr_print_infix_with_label(B, "B");
    printf("\n");

    void* rotated_v = expr_rotate_vector_in_plane(v, B, M_PI / 6);
    expr_print_infix_with_label(rotated_v, "rotated v");

    void* simp_rotated_v = expr_expand_reduce(rotated_v);
    expr_print_infix_with_label(simp_rotated_v, "simplified rotated v");

    void* expected  = expr_decode_infix_str("sqrt(3) * 0.5 * e1111 + sqrt(3) * 0.5 * e1112");
    void* evaluated = expr_evaluate_function(expected);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(simp_rotated_v, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(rotated_v);
    expr_free_tree(simp_rotated_v);
    expr_free_tree(v);
    expr_free_tree(a);
    expr_free_tree(b);
    expr_free_tree(B);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_node_arithmetic_op_count(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* json_str = "{'*':['v0','e1','v0','e5', 'e7', {'+':['v0','e1']}]}";
    printf("json_str: %s\n", json_str);
    void* expr_node = expr_decode_json_str(json_str);
    expr_print_infix_with_label(expr_node, "infix expr");

    u64 op_count = expr_node_arithmetic_op_count(expr_node);
    printf("op_count: %lu\n", op_count);

    u64 expected = 6;
    printf("expected: %lu\n", expected);

    u64 result = op_count == expected;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_node);
    printf("\n");
}

void test_expr_reverse_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "(e1 + e2) * e1 * e2 * e3";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* reversed_expr = expr_reverse(expr);
    void* simplified_reversed_expr = expr_expand_reduce(reversed_expr);
    expr_print_infix_with_label(reversed_expr, "reversed expr");
    expr_print_infix_with_label(simplified_reversed_expr, "simplified reversed expr");

    void* expected = expr_decode_infix_str("e3 * e2 * e1 * (e1 + e2)");
    void* simp_expected = expr_expand_reduce(expected);
    expr_print_infix_with_label(expected, "expected");
    expr_print_infix_with_label(simp_expected, "simplified expected");

    u64 result = expr_compare(simplified_reversed_expr, simp_expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(reversed_expr);
    expr_free_tree(simplified_reversed_expr);
    expr_free_tree(expected);
    expr_free_tree(simp_expected);
}

void test_expr_reverse_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "e2 * e3 + -1.00000 * e1 * e3";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* reversed_expr = expr_reverse(expr);
    void* simplified_reversed_expr = expr_expand_reduce(reversed_expr);
    expr_print_infix_with_label(reversed_expr, "reversed expr");
    expr_print_infix_with_label(simplified_reversed_expr, "simplified reversed expr");

    void* expected = expr_decode_infix_str("e3 * e2 * e1 * (e1 + e2)");
    void* simp_expected = expr_expand_reduce(expected);
    expr_print_infix_with_label(expected, "expected");
    expr_print_infix_with_label(simp_expected, "simplified expected");

    u64 result = expr_compare(simplified_reversed_expr, simp_expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(reversed_expr);
    expr_free_tree(simplified_reversed_expr);
    expr_free_tree(expected);
    expr_free_tree(simp_expected);
}

void test_expr_magnitude_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "e1";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* magnitude = expr_magnitude(expr);
    expr_print_infix_with_label(magnitude, "magnitude");

    void* expected = expr_create_scalar_node_with_f64(sqrt(1.0));
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(magnitude, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(magnitude);
    expr_free_tree(expected);
}

void test_expr_magnitude_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "e1 + 2";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* magnitude = expr_magnitude(expr);
    expr_print_infix_with_label(magnitude, "magnitude");

    void* expected = expr_create_scalar_node_with_f64(sqrt(5.0));
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(magnitude, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(magnitude);
    expr_free_tree(expected);
}

void test_expr_partial_derivative_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("2 * v1 + v1 * e1 + v1 * v2 + 3.14");
    expr_print_infix_with_label(expr, "expr");

    void* pd_expr = expr_partial_derivative(expr, 1);
    expr_print_infix_with_label(pd_expr, "pd_expr");
    void* simplified_pd_expr = expr_expand_reduce(pd_expr);
    expr_print_infix_with_label(simplified_pd_expr, "simplified pd_expr");

    void* expected = expr_decode_infix_str("2.0 + v2 + e1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_pd_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(pd_expr);
    expr_free_tree(simplified_pd_expr);
    expr_free_tree(expected);
}

void test_expr_partial_derivative_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("sin(v1)");
    expr_print_infix_with_label(expr, "expr");

    void* pd_expr = expr_partial_derivative(expr, 1);
    expr_print_infix_with_label(pd_expr, "pd_expr");
    void* simplified_pd_expr = expr_expand_reduce(pd_expr);
    expr_print_infix_with_label(simplified_pd_expr, "simplified pd_expr");

    void* expected = expr_decode_infix_str("cos(v1)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_pd_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(pd_expr);
    expr_free_tree(simplified_pd_expr);
    expr_free_tree(expected);
}

void test_expr_partial_derivative_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("sin(sin(v1))");
    expr_print_infix_with_label(expr, "expr");

    void* pd_expr = expr_partial_derivative(expr, 1);
    expr_print_infix_with_label(pd_expr, "pd_expr");
    void* simplified_pd_expr = expr_expand_reduce(pd_expr);
    expr_print_infix_with_label(simplified_pd_expr, "simplified pd_expr");

    void* expected = expr_decode_prefix_str("(* (cos (sin v1)) (cos v1))");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_pd_expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(pd_expr);
    expr_free_tree(simplified_pd_expr);
    expr_free_tree(expected);
}

void test_expr_partial_derivative_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("exp(sin(v1))");
    expr_print_infix_with_label(expr, "expr");

    void* pd1 = expr_partial_derivative(expr, 1);
    void* pd2 = expr_partial_derivative(pd1, 1);
    void* pd3 = expr_partial_derivative(pd2, 1);
    void* pd4 = expr_partial_derivative(pd3, 1);

    struct b64_kv_array* variable_values = b64_kv_array_create(2);

    union b64 key;
    union b64 value;

    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);

    void* pd4_evaluated = expr_evaluate(pd4, variable_values);
    expr_print_infix_with_label(pd4_evaluated, "pd4_evaluated");

    void* expected = expr_decode_prefix_str("0.94953001188472077");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(pd4_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(pd1);
    expr_free_tree(pd2);
    expr_free_tree(pd3);
    expr_free_tree(pd4);
    expr_free_tree(pd4_evaluated);
    expr_free_tree(expected);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_partial_derivative_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("exp(sin(v1))");
    expr_print_infix_with_label(expr, "expr");

    void* pd4 = expr_nth_partial_derivative(expr, 1, 4);

    struct b64_kv_array* variable_values = b64_kv_array_create(2);

    union b64 key;
    union b64 value;

    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);

    void* pd4_evaluated = expr_evaluate(pd4, variable_values);
    expr_print_infix_with_label(pd4_evaluated, "pd4_evaluated");

    void* expected = expr_decode_prefix_str("0.94953001188472077");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(pd4_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(pd4);
    expr_free_tree(pd4_evaluated);
    expr_free_tree(expected);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_vector_derivative_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* M = expr_decode_infix_str("v1 * e1 + v2 * e2 + (v1 * v1 + v2 * v2) * e3");
    expr_print_infix_with_label(M, "M");

    void* M_pd1 = expr_partial_derivative(M, 1);
    void* simp_M_pd1 = expr_expand_reduce(M_pd1);

    void* M_pd2 = expr_partial_derivative(M, 2);
    void* simp_M_pd2 = expr_expand_reduce(M_pd2);

    struct list* basis_vecs = list_create();
    list_append(basis_vecs, simp_M_pd1);
    list_append(basis_vecs, simp_M_pd2);

    struct list* reciprocal_basis_vecs = expr_reciprocal_basis(basis_vecs);

    void* F = expr_decode_infix_str("(v2 + 1) * log(v1)");
    expr_print_infix_with_label(F, "F");

    struct variadic_node* vec_derivative = expr_create_geometric_sum_node();
    for (u64 i = 0; i < reciprocal_basis_vecs->length; ++i)
    {
        void* reciprocal_vec = list_get(reciprocal_basis_vecs, i);
        struct variadic_node* vec_derivative_term = expr_create_geometric_product_node();
        expr_variadic_node_append_child(vec_derivative_term, reciprocal_vec);

        void* pd_F = expr_partial_derivative(F, i + 1);
        void* simp_pd_F = expr_expand_reduce(pd_F);
        expr_free_tree(pd_F);

        expr_variadic_node_append_child(vec_derivative_term, simp_pd_F);
        expr_variadic_node_append_child(vec_derivative, vec_derivative_term);
    }

    struct b64_kv_array* variable_values = b64_kv_array_create(2);

    union b64 key;
    union b64 value;

    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);

    key.u64_value = 2;
    value.f64_value = 0.0;
    b64_kv_array_set(variable_values, key, value);

    void* vec_derivative_evaluated = expr_evaluate(vec_derivative, variable_values);
    expr_print_infix_with_label(vec_derivative_evaluated, "vec_derivative_evaluated");

    void* expected = expr_decode_infix_str("0.2 * e1 + 0.4 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(vec_derivative_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    b64_kv_array_free(variable_values, 0);

    expr_free_tree(M);
    expr_free_tree(M_pd1);
    expr_free_tree(simp_M_pd1);
    expr_free_tree(M_pd2);
    expr_free_tree(simp_M_pd2);

    expr_free_tree(F);

    expr_free_tree(vec_derivative);
    expr_free_tree(vec_derivative_evaluated);
    expr_free_tree(expected);

    list_free(basis_vecs, 0);
    list_free(reciprocal_basis_vecs, 0);
}

void test_expr_vector_derivative_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* F = expr_decode_infix_str("(v2 + 1) * log(v1)");
    expr_print_infix_with_label(F, "F");

    void* M = expr_decode_infix_str("v1 * e1 + v2 * e2 + (v1 * v1 + v2 * v2) * e3");
    expr_print_infix_with_label(M, "M");

    void* vd = expr_vector_derivative(F, M, 2);

    struct b64_kv_array* variable_values = b64_kv_array_create(2);
    union b64 key;
    union b64 value;
    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);
    key.u64_value = 2;
    value.f64_value = 0.0;
    b64_kv_array_set(variable_values, key, value);

    void* vd_func_evaluated = expr_evaluate(vd, variable_values);

    expr_print_infix_with_label(vd_func_evaluated, "vd_func_evaluated");

    void* expected = expr_decode_infix_str("0.2 * e1 + 0.4 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(vd_func_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(F);
    expr_free_tree(M);
    expr_free_tree(vd);
    expr_free_tree(vd_func_evaluated);
    expr_free_tree(expected);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_vector_derivative_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* F = expr_decode_infix_str("v1 * e1 + v2 * e2 + v3 * e3");
    expr_print_infix_with_label(F, "F");

    void* M = expr_decode_infix_str("v1 * e1 + v2 * e2 + v3 * e3");
    expr_print_infix_with_label(M, "M");

    void* vd = expr_vector_derivative(F, M, 3);
    void* vd_func_evaluated = expr_evaluate_function(vd);
    expr_print_infix_with_label(vd_func_evaluated, "vd_func_evaluated");

    void* expected = expr_decode_infix_str("3.0");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(vd_func_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(F);
    expr_free_tree(M);
    expr_free_tree(vd);
    expr_free_tree(vd_func_evaluated);
    expr_free_tree(expected);
}

void test_expr_vector_derivative_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* M = expr_decode_infix_str("v1 * e1 + v2 * e2 + (v1 * v1 + v2 * v2) * e3");
    expr_print_infix_with_label(M, "M");

    void* M_pd_v1 = expr_partial_derivative(M, 1);
    void* simp_M_pd_v1 = expr_expand_reduce(M_pd_v1);
    expr_free_tree(M_pd_v1);

    void* M_pd_v2 = expr_partial_derivative(M, 2);
    void* simp_M_pd_v2 = expr_expand_reduce(M_pd_v2);
    expr_free_tree(M_pd_v2);

    expr_print_infix_with_label(simp_M_pd_v1, "simp_M_pd_v1");
    expr_print_infix_with_label(simp_M_pd_v2, "simp_M_pd_v2");

    void* v2_plus_1 = expr_decode_infix_str("v2 + 1");
    void* v2_plus_1_M_pd_v1 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(v2_plus_1_M_pd_v1, v2_plus_1);
    expr_variadic_node_append_child(v2_plus_1_M_pd_v1, simp_M_pd_v1);

    void* v1_squared = expr_decode_infix_str("v1 * v1");
    void* v1_squared_M_pd_v2 = expr_create_geometric_product_node();
    expr_variadic_node_append_child(v1_squared_M_pd_v2, v1_squared);
    expr_variadic_node_append_child(v1_squared_M_pd_v2, simp_M_pd_v2);

    void* F = expr_create_geometric_sum_node();
    expr_variadic_node_append_child(F, v2_plus_1_M_pd_v1);
    expr_variadic_node_append_child(F, v1_squared_M_pd_v2);
    expr_print_infix_with_label(F, "F");

    void* vd = expr_vector_derivative(F, M, 2);

    struct b64_kv_array* variable_values = b64_kv_array_create(2);
    union b64 key;
    union b64 value;
    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);
    key.u64_value = 2;
    value.f64_value = 0.0;
    b64_kv_array_set(variable_values, key, value);

    void* vd_evaluated = expr_evaluate(vd, variable_values);

    expr_print_infix_with_label(vd_evaluated, "vd_func_evaluated");

    void* expected = expr_decode_infix_str("0.8 + (0.4 * e1 * e3) + (-0.6 * e1 * e2) + (3.2 * e2 * e3)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(vd_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(F);
    expr_free_tree(M);
    expr_free_tree(vd);
    expr_free_tree(vd_evaluated);
    expr_free_tree(expected);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_vector_derivative_n_decode_infix(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* F = expr_decode_infix_str("(v2 + 1) * log(v1)");
    expr_print_infix_with_label(F, "F");

    void* M = expr_decode_infix_str("v1 * e1 + v2 * e2 + (v1 * v1 + v2 * v2) * e3");
    expr_print_infix_with_label(M, "M");

    void* vd = expr_vector_derivative(F, M, 2);
    expr_print_infix_with_label(vd, "vd");

    // decode the output of expr_print_infix_with_label(vd, "vd");
    void* vd_ = expr_decode_infix_str("(((((reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) + (4.00000 * v2 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1) + (((2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) + (-2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1 * e2 * e3) + ((2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) * e3) + ((-4.00000 * v1 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) * e2)) * (((0.00000 + 0.00000) * log(v1)) + ((v2 + 1.00000) * (reciprocal(v1) * 1.00000)))) + ((((reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) + (4.00000 * v1 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e2) + ((2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) * e3) + (((-2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) + (2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1 * e2 * e3) + ((-4.00000 * v2 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) * e1)) * (((1.00000 + 0.00000) * log(v1)) + ((v2 + 1.00000) * (reciprocal(v1) * 0.00000)))))");
    expr_print_infix_with_label(vd_, "vd_");

    // decode the output of expr_print_infix_with_label(vd_, "vd_");
    void* vd__ = expr_decode_infix_str("(((((reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) + (4.00000 * v2 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1) + (((2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) + (-2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1 * e2 * e3) + (2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) * e3) + (-4.00000 * v1 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) * e2)) * (((0.00000 + 0.00000) * log(v1)) + ((v2 + 1.00000) * reciprocal(v1) * 1.00000))) + ((((reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) + (4.00000 * v1 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e2) + (2.00000 * v2 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) * e3) + (((-2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1)))) + (2.00000 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))))) * e1 * e2 * e3) + (-4.00000 * v2 * v1 * reciprocal((1.00000 + (4.00000 * v2 * v2) + (4.00000 * v1 * v1))) * e1)) * (((1.00000 + 0.00000) * log(v1)) + ((v2 + 1.00000) * reciprocal(v1) * 0.00000))))");
    expr_print_infix_with_label(vd__, "vd__");

    struct b64_kv_array* variable_values = b64_kv_array_create(2);
    union b64 key;
    union b64 value;
    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);
    key.u64_value = 2;
    value.f64_value = 0.0;
    b64_kv_array_set(variable_values, key, value);

    void* vd_evaluated = expr_evaluate(vd__, variable_values);

    expr_print_infix_with_label(vd_evaluated, "vd_func_evaluated");

    void* expected = expr_decode_infix_str("0.2 * e1 + 0.4 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(vd_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(F);
    expr_free_tree(M);
    expr_free_tree(vd);
    expr_free_tree(vd_);
    expr_free_tree(vd__);
    expr_free_tree(vd_evaluated);
    expr_free_tree(expected);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_gradient_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "2 * v1 + v1 * e1 + v1 * v2 + 3.14";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* grad = expr_gradient(expr, 2);
    expr_print_infix_with_label(grad, "grad");
    void* simplified_grad = expr_expand_reduce(grad);
    expr_print_infix_with_label(simplified_grad, "simplified grad");

    void* expected = expr_decode_infix_str("1.0 + (2.0 + v2) * e1 + v1 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_grad, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(grad);
    expr_free_tree(simplified_grad);
    expr_free_tree(expected);
}

void test_expr_gradient_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "(v1 * e1 + v2 * e2) * (v1 * e1 + v2 * e2)";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* grad = expr_gradient(expr, 2);
    expr_print_infix_with_label(grad, "grad");
    void* simplified_grad = expr_expand_reduce(grad);
    expr_print_infix_with_label(simplified_grad, "simplified grad");

    void* expected = expr_decode_infix_str("2.0 * v1 * e1 + 2.0 * v2 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_grad, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(grad);
    expr_free_tree(simplified_grad);
    expr_free_tree(expected);
}

void test_expr_basis(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* basis = list_create();
    for (u64 i = 1; i < 6; ++i)
    {
        list_append(basis, expr_create_basis_vector_node(i));
    }

    expr_print_basis(basis);

    u64 result = 1;

    for (u64 i = 1; i < 6; ++i)
    {
        struct leaf_node* basis_vector = list_get(basis, i - 1);
        result = result && expr_is_basis_vector_node(basis_vector) && (basis_vector->dim_index == i);
    }

    for (u64 i = 1; i < 6; ++i)
    {
        expr_free_tree(list_get(basis, i - 1));
    }

    list_free(basis, 0);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }
}

void test_expr_grade_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "3.14";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    u64 grade = expr_grade_of_simplified_term(expr);
    printf("grade: %lu\n", grade);

    u64 expected = 0;

    u64 result = grade == expected;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
}

void test_expr_grade_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "e1";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    u64 grade = expr_grade_of_simplified_term(expr);
    printf("grade: %lu\n", grade);

    u64 expected = 1;

    u64 result = grade == expected;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
}

void test_expr_grade_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "3.14 * e1 * e3";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    u64 grade = expr_grade_of_simplified_term(expr);
    printf("grade: %lu\n", grade);

    u64 expected = 2;

    u64 result = grade == expected;

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
}

void test_expr_k_vector_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "(3.14 + v1) * e1 * e3 + (v2 + v3) * e2 * e3 + e1 + 2.718 + e3";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* k_vector = expr_k_vector_of_simplified_expr(expr, 0);
    expr_print_infix_with_label(k_vector, "k_vector");

    void* expected = expr_create_scalar_node_with_f64(2.718);
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(k_vector, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(k_vector);
    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_expr_k_vector_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    char* infix_expr = "(3.14 + v1) * e1 * e3 + (v2 + v3) * e2 * e3 + e1 + 2.718 + e3";
    void* expr = expr_decode_infix_str(infix_expr);
    expr_print_infix_with_label(expr, "expr");

    void* k_vector = expr_k_vector_of_simplified_expr(expr, 2);
    expr_print_infix_with_label(k_vector, "k_vector");

    void* expected = expr_decode_infix_str("(3.14000 + v1) * e1 * e3 + (v2 + v3) * e2 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(k_vector, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(k_vector);
    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_expr_scalar_product(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr_1 = expr_decode_infix_str("e1 + e2 + 3");
    void* expr_2 = expr_decode_infix_str("e1 + e3 + v1");

    void* scalar_product = expr_scalar_product(expr_1, expr_2);
    expr_print_infix_with_label(scalar_product, "scalar_product");

    void* expected = expr_decode_infix_str("1.0 + 3.0 * v1");

    u64 result = expr_compare(scalar_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_1);
    expr_free_tree(expr_2);
    expr_free_tree(scalar_product);
    expr_free_tree(expected);
}

void test_expr_inner_product_of_simplified_terms(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr_1 = expr_decode_infix_str("e1");
    void* expr_2 = expr_decode_infix_str("e1");

    void* inner_product = expr_inner_product_of_simplied_terms(expr_1, expr_2);
    expr_print_infix_with_label(inner_product, "inner_product");

    void* expected = expr_decode_infix_str("1.0");

    u64 result = expr_compare(inner_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_1);
    expr_free_tree(expr_2);
    expr_free_tree(inner_product);
    expr_free_tree(expected);
}

void test_expr_inner_product(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1 + e2");
    void* expr_2 = expr_decode_infix_str("e1 * e2");
    void* expr_3 = expr_decode_infix_str("e1 * e2 * e3");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);
    list_append(exprs, expr_3);

    void* inner_product = expr_inner_product(exprs);
    expr_print_infix_with_label(inner_product, "inner_product");

    void* expected = expr_decode_infix_str("-1.0 * e1 * e3 + -1 * e2 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(inner_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    expr_print_infix_with_label(inner_product, "inner_product");
    expr_free_tree(inner_product);
    expr_free_tree(expected);
}

void test_expr_outer_product_of_simplified_terms(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr_1 = expr_decode_infix_str("e1");
    void* expr_2 = expr_decode_infix_str("e2");

    void* outer_product = expr_outer_product_of_simplied_terms(expr_1, expr_2);
    expr_print_infix_with_label(outer_product, "outer_product");

    void* expected = expr_decode_infix_str("e1 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(outer_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr_1);
    expr_free_tree(expr_2);
    expr_free_tree(outer_product);
    expr_free_tree(expected);
}

void test_expr_outer_product_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1 + e3");
    void* expr_2 = expr_decode_infix_str("e2 + e3");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);

    void* outer_product = expr_outer_product(exprs);
    expr_print_infix_with_label(outer_product, "outer_product");

    void* expected = expr_decode_infix_str("e1 * e2 + e1 * e3 + -1.0 * e2 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(outer_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    expr_print_infix_with_label(outer_product, "outer_product");
    expr_free_tree(outer_product);
    expr_free_tree(expected);
}

void test_expr_outer_product_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1 + e3");
    void* expr_2 = expr_decode_infix_str("e2 + e3");
    void* expr_3 = expr_decode_infix_str("e4 * e5 * e6");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);
    list_append(exprs, expr_3);

    void* outer_product = expr_outer_product(exprs);
    expr_print_infix_with_label(outer_product, "outer_product");

    void* expected = expr_decode_infix_str("e1 * e2 * e4 * e5 * e6 + e1 * e3 * e4 * e5 * e6 + -1.0 * e2 * e3 * e4 * e5 * e6");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(outer_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    expr_print_infix_with_label(outer_product, "outer_product");
    expr_free_tree(outer_product);
    expr_free_tree(expected);
}

void test_expr_commutator_product(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr_1 = expr_decode_infix_str("e1 + e2");
    void* expr_2 = expr_decode_infix_str("e1");

    void* commutator_product = expr_commutator_product(expr_1, expr_2);
    expr_print_infix_with_label(commutator_product, "commutator_product");

    void* expected = expr_decode_infix_str("-1.0 * e1 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(commutator_product, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr_1);
    expr_free_tree(expr_2);
    expr_free_tree(commutator_product);
    expr_free_tree(expected);
}

void test_expr_assign_values_variables(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("v1 + v2 + 2 * v3 * v4");
    expr_print_infix_with_label(expr, "expr");

    struct b64_kv_array* variable_values = b64_kv_array_create(4);

    union b64 key;
    union b64 value;

    key.u64_value = 1;
    value.f64_value = 1.0;
    b64_kv_array_set(variable_values, key, value);

    key.u64_value = 2;
    value.f64_value = 2.0;
    b64_kv_array_set(variable_values, key, value);

    key.u64_value = 3;
    value.f64_value = 3.0;
    b64_kv_array_set(variable_values, key, value);

    key.u64_value = 4;
    value.f64_value = 4.0;
    b64_kv_array_set(variable_values, key, value);

    void* expr_ = expr_assign_values_variables(expr, variable_values);
    expr_print_infix_with_label(expr_, "expr_");
    f64 scalar = expr_numerical_value(expr_);
    printf("value: %f\n", scalar);

    f64 expected = 1.0 + 2.0 + 2.0 * 3.0 * 4.0;
    printf("expected: %f\n", expected);
    print_block_line(scalar == expected ? "Passed" : "Failed");

    if (scalar == expected)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expr_);
    b64_kv_array_free(variable_values, 0);
}

void test_expr_inverse(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("e1 * e2");

    void* inverse = expr_inverse_blade(expr);
    expr_print_infix_with_label(inverse, "inverse");

    void* evaluated = expr_evaluate_function(inverse);
    expr_print_infix_with_label(evaluated, "simplified result");

    void* expected = expr_decode_infix_str("-1.0 * e1 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    print_block_line(result ? "Passed" : "Failed");

    expr_free_tree(expected);
    expr_free_tree(evaluated);
    expr_free_tree(inverse);
    expr_free_tree(expr);
}

void test_expr_project(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("e1 + e2 + 2 * e3");
    void* A = expr_decode_infix_str("e1 * e2");

    void* project = expr_project(expr, A);

    void* evaluated = expr_evaluate_function(project);
    expr_print_infix_with_label(evaluated, "project");

    void* expected = expr_decode_infix_str("e1 + e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    print_block_line(result ? "Passed" : "Failed");

    expr_free_tree(expr);
    expr_free_tree(A);
    expr_free_tree(project);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
}

void test_expr_reject(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("e1 + e2 + 2 * e3");
    void* A = expr_decode_infix_str("e1 * e2");

    void* reject = expr_reject(expr, A);

    void* evaluated = expr_evaluate_function(reject);
    expr_print_infix_with_label(evaluated, "reject");

    void* expected = expr_decode_infix_str("2 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    print_block_line(result ? "Passed" : "Failed");

    expr_free_tree(expr);
    expr_free_tree(A);
    expr_free_tree(reject);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
}

void test_expr_reciprocal_basis_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1");
    void* expr_2 = expr_decode_infix_str("e1 + e2");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);

    void* expected_expr_1 = expr_decode_infix_str("-1.0 * e2 + e1");
    void* expected_expr_2 = expr_decode_infix_str("e2");

    struct list* reciprocal_basis = expr_reciprocal_basis(exprs);
    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    void* rb_1 = list_get(reciprocal_basis, 0);
    void* evaluated_1 = expr_evaluate_function(rb_1);

    void* rb_2 = list_get(reciprocal_basis, 1);
    void* evaluated_2 = expr_evaluate_function(rb_2);

    u64 result = expr_compare(evaluated_1, expected_expr_1) &&
                 expr_compare(evaluated_2, expected_expr_2);

    expr_print_infix_with_label(evaluated_1, "reciprocal basis 1");
    expr_print_infix_with_label(evaluated_2, "reciprocal basis 2");

    expr_print_infix_with_label(expected_expr_1, "expected_expr_1");
    expr_print_infix_with_label(expected_expr_2, "expected_expr_2");


    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(rb_1);
    expr_free_tree(rb_2);
    expr_free_tree(evaluated_1);
    expr_free_tree(evaluated_2);
    expr_free_tree(expected_expr_1);
    expr_free_tree(expected_expr_2);

    list_free(reciprocal_basis, 0);
    printf("\n");
}

void test_expr_reciprocal_basis_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1");
    void* expr_2 = expr_decode_infix_str("e1 + e2");
    void* expr_3 = expr_decode_infix_str("e3");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);
    list_append(exprs, expr_3);

    struct list* reciprocal_basis = expr_reciprocal_basis(exprs);
    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    void* rb_1 = list_get(reciprocal_basis, 0);
    void* evaluated_1 = expr_evaluate_function(rb_1);

    void* rb_2 = list_get(reciprocal_basis, 1);
    void* evaluated_2 = expr_evaluate_function(rb_2);

    void* rb_3 = list_get(reciprocal_basis, 2);
    void* evaluated_3 = expr_evaluate_function(rb_3);

    void* expected_expr_1 = expr_decode_infix_str("-1.0 * e2 + e1");
    void* expected_expr_2 = expr_decode_infix_str("e2");
    void* expected_expr_3 = expr_decode_infix_str("e3");

    u64 result = expr_compare(evaluated_1, expected_expr_1) &&
                 expr_compare(evaluated_2, expected_expr_2) &&
                 expr_compare(evaluated_3, expected_expr_3);


    expr_print_infix_with_label(evaluated_1, "reciprocal basis 1");
    expr_print_infix_with_label(evaluated_2, "reciprocal basis 2");
    expr_print_infix_with_label(evaluated_3, "reciprocal basis 3");

    expr_print_infix_with_label(expected_expr_1, "expected_expr_1");
    expr_print_infix_with_label(expected_expr_2, "expected_expr_2");
    expr_print_infix_with_label(expected_expr_3, "expected_expr_3");

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(rb_1);
    expr_free_tree(rb_2);
    expr_free_tree(rb_3);

    expr_free_tree(evaluated_1);
    expr_free_tree(evaluated_2);
    expr_free_tree(evaluated_3);

    expr_free_tree(expected_expr_1);
    expr_free_tree(expected_expr_2);
    expr_free_tree(expected_expr_3);

    list_free(reciprocal_basis, 0);

    printf("\n");
}

void test_expr_reciprocal_basis_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct list* exprs = list_create();

    void* expr_1 = expr_decode_infix_str("e1 + 2 * e3");
    void* expr_2 = expr_decode_infix_str("e2");

    list_append(exprs, expr_1);
    list_append(exprs, expr_2);

    struct list* reciprocal_basis = expr_reciprocal_basis(exprs);
    for (u64 i = 0; i < exprs->length; ++i)
    {
        expr_free_tree(list_get(exprs, i));
    }
    list_free(exprs, 0);

    void* rb_1 = list_get(reciprocal_basis, 0);
    void* evaluated_1 = expr_evaluate_function(rb_1);

    void* rb_2 = list_get(reciprocal_basis, 1);
    void* evaluated_2 = expr_evaluate_function(rb_2);

    void* expected_expr_1 = expr_decode_infix_str("0.2 * e1 + 0.4 * e3");
    void* expected_expr_2 = expr_decode_infix_str("e2");

    u64 result = expr_compare(evaluated_1, expected_expr_1) &&
                 expr_compare(evaluated_2, expected_expr_2);


    expr_print_infix_with_label(evaluated_1, "reciprocal basis 1");
    expr_print_infix_with_label(evaluated_2, "reciprocal basis 2");

    expr_print_infix_with_label(expected_expr_1, "expected_expr_1");
    expr_print_infix_with_label(expected_expr_2, "expected_expr_2");

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(rb_1);
    expr_free_tree(rb_2);

    expr_free_tree(evaluated_1);
    expr_free_tree(evaluated_2);

    expr_free_tree(expected_expr_1);
    expr_free_tree(expected_expr_2);

    list_free(reciprocal_basis, 0);
    printf("\n");
}

void test_decode_function_infix_expr(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("sqrt(reciprocal(sqrt(e1 + e2)))");
    expr_print_infix_with_label(expr, "infix expr");

    void* sum = expr_decode_infix_str("e1 + e2");
    void* sqrt = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(sqrt, sum);

    void* reciprocal = expr_create_function_node("reciprocal");
    expr_variadic_node_prepend_child(reciprocal, sqrt);

    void* expected = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(expected, reciprocal);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_decode_function_prefix_expr(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_prefix_str("(sqrt (reciprocal (sqrt (+ e1 e2))))");
    expr_print_infix_with_label(expr, "prefix expr");

    void* sum = expr_decode_prefix_str("(+ e1 e2)");
    void* sqrt = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(sqrt, sum);

    void* reciprocal = expr_create_function_node("reciprocal");
    expr_variadic_node_prepend_child(reciprocal, sqrt);

    void* expected = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(expected, reciprocal);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_decode_function_json_expr(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_json_str("{'sqrt':[{'reciprocal':[{'sqrt':[{'+':['e1','e2']}]}]}]}");
    expr_print_infix_with_label(expr, "expr");

    void* sum = expr_decode_json_str("{'+':['e1','e2']}");
    void* sqrt = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(sqrt, sum);

    void* reciprocal = expr_create_function_node("reciprocal");
    expr_variadic_node_prepend_child(reciprocal, sqrt);

    void* expected = expr_create_function_node("sqrt");
    expr_variadic_node_prepend_child(expected, reciprocal);

    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
}

void test_expr_function_node(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* func = expr_create_function_node("sqrt");
    void* expr = expr_decode_infix_str("e1 + e2");
    expr_variadic_node_prepend_child(func, expr);

    void* simplified_func = expr_expand_reduce(func);

    expr_print_infix_with_label(func, "func");
    expr_print_infix_with_label(simplified_func, "simplified_func");

    void* expected = expr_decode_infix_str("sqrt(e1 + e2)");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(simplified_func, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(func);
    expr_free_tree(simplified_func);
    expr_free_tree(expected);
}

void test_expr_evaluate_function_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    // sqrt(reciprocal(2.00000))
    void* root = expr_create_function_node("sqrt");
    void* reciprocal = expr_create_function_node("reciprocal");
    expr_variadic_node_append_child(reciprocal, expr_create_scalar_node_with_f64(2.0));
    expr_variadic_node_prepend_child(root, reciprocal);

    void* evaluated = expr_evaluate_function(root);
    f64 scalar = expr_numerical_value(evaluated);
    f64 expected = sqrt(1.0 / 2.0);

    expr_print_infix_with_label(root, "root");

    printf("scalar: %f\n", scalar);
    printf("expected: %f\n", expected);

    if (scalar == expected)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    print_block_line(scalar == expected ? "Passed" : "Failed");

    expr_free_tree(root);
    expr_free_tree(evaluated);
}

void test_expr_evaluate_function_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("sin(reciprocal(2) * 3.14159265359)");
    expr_print_infix_with_label(expr, "expr");

    void* evaluated = expr_evaluate_function(expr);
    expr_print_tree_with_label(evaluated, "evaluated");

    void* expected = expr_create_scalar_node_with_f64(sin(1.0 / 2.0 * 3.14159265359));
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
}

void test_expr_evaluate_function_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("(reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000)) + 4.00000 * 0.00000 * 0.00000 * reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000))) * e1 + (2.00000 * 0.00000 * reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000)) + -2.00000 * 0.00000 * reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000))) * e1 * e2 * e3 + (2.00000 * 1.00000 * reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000))) * e3 + (-4.00000 * 1.00000 * 0.00000 * reciprocal((1.00000 + 4.00000 * 0.00000 * 0.00000 + 4.00000 * 1.00000 * 1.00000))) * e2");
    expr_print_infix_with_label(expr, "expr");

    void* func_evaluated = expr_evaluate_function(expr);
    expr_print_infix_with_label(func_evaluated, "func evaluated");

    void* expected = expr_decode_infix_str("0.2 * e1 + 0.4 * e3");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(func_evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(func_evaluated);
    expr_free_tree(expected);
    printf("\n");
}

void test_expr_evaluate_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_decode_infix_str("1.0 + sin(reciprocal(2 + 1.0) * 3.14159265359)");
    expr_print_infix_with_label(expr, "expr");

    void* evaluated = expr_evaluate(expr, NULL);
    expr_print_infix_with_label(evaluated, "evaluated");

    void* expected = expr_create_scalar_node_with_f64(1.0 + sin(3.14159265359 / 3.0));
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);
    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
}

void test_expr_clone_tree(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct expr_node* expr = expr_decode_infix_str("sin(v1)");
    struct expr_node* clone = expr_clone_tree(expr);
    expr_print_infix_with_label(expr, "expr");
    expr_print_infix_with_label(clone, "clone");

    u64 result = expr_compare(expr, clone);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(clone);
}

void test_expr_remove_redundant_nodes_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    struct expr_node* expr = expr_decode_prefix_str("(* (+ (* ) v2) )");
    void* expr_ = expr_remove_redundant_nodes(expr);

    expr_print_tree_with_label(expr, "expr");
    expr_print_tree_with_label(expr_, "expr_");

    void* expected = expr_decode_prefix_str("v2");

    u64 result = expr_compare(expr_, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {
        ++passed_test_count;
    }
    else
    {
        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expr_);
    expr_free_tree(expected);
}

void test_expr_exp_mv3_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(1.0, // a0
                              0.0, 0.0, 0.0, // a1, a2, a3
                              0.0, 0.0, 0.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("exp(1)");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(0.0, // a0
                              1.0, 0.0, 0.0, // a1, a2, a3
                              0.0, 0.0, 0.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("cosh(1) + sinh(1) * e1");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(0.0, // a0
                              1.0, 1.0, 1.0, // a1, a2, a3
                              0.0, 0.0, 0.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("cosh(sqrt(3)) + sinh(sqrt(3)) * reciprocal(sqrt(3)) * (e1 + e2 + e3)");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(0.0, // a0
                              0.0, 0.0, 0.0, // a1, a2, a3
                              1.0, 0.0, 0.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("cos(1) + sin(1) *e1 * e2");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(0.0, // a0
                              0.0, 0.0, 0.0, // a1, a2, a3
                              1.0, 1.0, 1.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("cos(sqrt(3)) + sin(sqrt(3)) * reciprocal(sqrt(3)) * (e1 * e2 + e1 * e3 + e2 * e3)");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_6(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(1.0, // a0
                              0.0, 0.0, 0.0, // a1, a2, a3
                              0.0, 0.0, 0.0, // a12, a13, a23
                              1.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("exp(1) * cos(1) + exp(1) * sin(1) * e1 * e2 * e3");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_mv3_7(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* expr = expr_exp_mv3(0.0, // a0
                              1.0, 0.0, 0.0, // a1, a2, a3
                              1.0, 0.0, 0.0, // a12, a13, a23
                              0.0 // a123
                 );
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("1 + e1 + e1 * e2");
    void* evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(evaluated, "expected");

    u64 result = expr_compare(expr, evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(expr);
    expr_free_tree(expected);
    expr_free_tree(evaluated);
    printf("\n");
}

void test_expr_exp_3d_1(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* input = expr_decode_infix_str("pi() * e1 * e2");
    void* expr = expr_exp_3d(input);
    expr_print_infix_with_label(expr, "expr");

    void* expected = expr_decode_infix_str("-1");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(expr, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(input);
    expr_free_tree(expr);
    expr_free_tree(expected);
    printf("\n");
}

void test_expr_exp_3d_2(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* _R = expr_decode_infix_str("-0.25 * pi() * e1 * e2");
    void* R = expr_decode_infix_str("0.25 * pi() * e1 * e2");

    void* _R_exp = expr_exp_3d(_R);
    void* R_exp = expr_exp_3d(R);
    void* input = expr_decode_infix_str("e1 + e2");
    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, _R_exp);
    expr_variadic_node_append_child(expr, input);
    expr_variadic_node_append_child(expr, R_exp);
    void* evaluated = expr_evaluate(expr, NULL);

    expr_print_infix_with_label(evaluated, "evaluated");

    void* expected = expr_decode_infix_str("-1 * e1 + e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(_R);
    expr_free_tree(R);
    expr_free_tree(expr);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
    printf("\n");
}

void test_expr_exp_3d_3(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* _R = expr_decode_infix_str("-0.5 * pi() * e1 * e2");
    void* R = expr_decode_infix_str("0.5 * pi() * e1 * e2");

    void* _R_exp = expr_exp_3d(_R);
    void* R_exp = expr_exp_3d(R);
    void* input = expr_decode_infix_str("e1 + e2");
    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, _R_exp);
    expr_variadic_node_append_child(expr, input);
    expr_variadic_node_append_child(expr, R_exp);
    void* evaluated = expr_evaluate(expr, NULL);

    expr_print_infix_with_label(evaluated, "evaluated");

    void* expected = expr_decode_infix_str("-1 * e1 + -1 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(_R);
    expr_free_tree(R);
    expr_free_tree(expr);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
    printf("\n");
}

void test_expr_exp_3d_4(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* _R = expr_decode_infix_str("0.25 * pi() * e1 * e2");
    void* R = expr_decode_infix_str("-0.25 * pi() * e1 * e2");

    void* _R_exp = expr_exp_3d(_R);
    void* R_exp = expr_exp_3d(R);
    void* input = expr_decode_infix_str("3 * e1 + 4 * e2");
    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, _R_exp);
    expr_variadic_node_append_child(expr, input);
    expr_variadic_node_append_child(expr, R_exp);
    void* evaluated = expr_evaluate(expr, NULL);

    expr_print_infix_with_label(evaluated, "evaluated");

    void* expected = expr_decode_infix_str("4 * e1 + -3 * e2");
    expr_print_infix_with_label(expected, "expected");

    u64 result = expr_compare(evaluated, expected);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(_R);
    expr_free_tree(R);
    expr_free_tree(expr);
    expr_free_tree(evaluated);
    expr_free_tree(expected);
    printf("\n");
}

void test_expr_exp_3d_5(u64 run)
{
    if(!test_filter(run))
    {
        return;
    }

    print_block_line(__func__);

    void* _R = expr_decode_infix_str("-0.375 * pi() * e1 * e2");
    void* R = expr_decode_infix_str("0.375 * pi() * e1 * e2"); // 0.375 = 3/8

    void* _R_exp = expr_exp_3d(_R);
    void* R_exp = expr_exp_3d(R);
    void* input = expr_decode_infix_str("3 * e2");
    void* expr = expr_create_geometric_product_node();
    expr_variadic_node_append_child(expr, _R_exp);
    expr_variadic_node_append_child(expr, input);
    expr_variadic_node_append_child(expr, R_exp);
    void* expr_evaluated = expr_evaluate(expr, NULL);

    expr_print_infix_with_label(expr_evaluated, "expr_evaluated");

    void* expected = expr_decode_infix_str("-1 * sqrt(4.5) * e2 + -1 * sqrt(4.5) * e1");
    void* expected_evaluated = expr_evaluate(expected, NULL);
    expr_print_infix_with_label(expected_evaluated, "expected");

    u64 result = expr_compare(expr_evaluated, expected_evaluated);

    print_block_line(result ? "Passed" : "Failed");

    if (result)
    {

        ++passed_test_count;
    }
    else
    {

        ++failed_test_count;
    }

    expr_free_tree(_R);
    expr_free_tree(R);
    expr_free_tree(expr);
    expr_free_tree(expr_evaluated);
    expr_free_tree(expected);
    expr_free_tree(expected_evaluated);
    printf("\n");
}

int main(int argc, char* argv[])
{
#ifdef DEBUG
    printf("Running in DEBUG mode\n");
#else
    printf("Running in release mode\n");
#endif

    force_run_all_tests = cli_check_flag(argc, argv, "--run-all");

    runtime_initialize();
    func_register("partial_derivative", func_partial_derivative, NULL, FUNC_FLAG_NONE, NULL);

    test_vec_a(0);

    test_decode_json_expr_1(0);
    test_decode_json_expr_2(0);
    test_decode_json_expr_3(0);
    test_decode_json_expr_4(0);

    test_encode_json_expr_to_json_1(0);
    test_encode_json_expr_to_json_2(0);
    test_encode_json_expr_to_json_3(0);
    test_encode_json_expr_to_json_4(0);

    test_expr_decode_prefix_str_1(0);
    test_expr_decode_prefix_str_2(0);
    test_expr_decode_prefix_str_3(0);
    test_expr_decode_prefix_str_4(0);
    test_expr_decode_prefix_str_5(0);

    test_expr_decode_infix_str_1(0);
    test_expr_decode_infix_str_2(0);
    test_expr_decode_infix_str_3(0);
    test_expr_decode_infix_str_4(0);
    test_expr_decode_infix_str_5(0);
    test_expr_decode_infix_str_6(0);
    test_expr_decode_infix_str_7(0);

    test_reciprocal_scalar_function_node(0);

    test_flatten_1(0);
    test_flatten_2(0);
    test_flatten_3(0);
    test_flatten_4(0);
    test_flatten_5(0);
    test_flatten_6(0);
    test_flatten_7(0);
    test_flatten_8(0);

    test_expr_variadic_node_append_child(0);
    test_expr_variadic_node_replace_child(0);
    test_expr_variadic_node_remove_child(0);

    test_expand_expr_product_node_1(0);
    test_expand_expr_product_node_2(0);

    test_expand_expr_sum_node_1(0);
    test_expand_expr_sum_node_2(0);
    test_expand_expr_sum_node_3(0);
    test_expand_expr_sum_node_4(0);

    test_expr_reduce_product_node_1(0);
    test_expr_reduce_product_node_2(0);
    test_expr_reduce_product_node_3(0);

    test_expr_reduce_sum_node_1(0);
    test_expr_reduce_sum_node_2(0);
    test_expr_reduce_sum_node_3(0);
    test_expr_reduce_sum_node_4(0);

    test_expand_reduce_expr_1(0);
    test_expand_reduce_expr_2(0);
    test_expand_reduce_expr_3(0);
    test_expand_reduce_expr_from_file(argv[0], 0);

    test_expr_rotate_vector_in_plane_1(0);
    test_expr_rotate_vector_in_plane_2(0);
    test_expr_rotate_vector_in_plane_3(0);

    test_expr_node_arithmetic_op_count(0);

    test_expr_reverse_1(0);
    test_expr_reverse_2(0);

    test_expr_magnitude_1(0);
    test_expr_magnitude_2(0);

    test_expr_partial_derivative_1(0);
    test_expr_partial_derivative_2(0);
    test_expr_partial_derivative_3(0);
    test_expr_partial_derivative_4(0);
    test_expr_partial_derivative_5(0);

    test_expr_vector_derivative_1(0);
    test_expr_vector_derivative_2(0);
    test_expr_vector_derivative_3(0);
    test_expr_vector_derivative_4(0);

    test_expr_vector_derivative_n_decode_infix(0);

    test_expr_gradient_1(0);
    test_expr_gradient_2(0);

    test_expr_basis(0);

    test_expr_grade_1(0);
    test_expr_grade_2(0);
    test_expr_grade_3(0);

    test_expr_k_vector_1(0);
    test_expr_k_vector_2(0);

    test_expr_scalar_product(0);

    test_expr_inner_product_of_simplified_terms(0);
    test_expr_inner_product(0);

    test_expr_outer_product_of_simplified_terms(0);
    test_expr_outer_product_1(0);
    test_expr_outer_product_2(0);

    test_expr_commutator_product(0);

    test_expr_reciprocal_basis_1(0);
    test_expr_reciprocal_basis_2(0);
    test_expr_reciprocal_basis_3(0);

    test_expr_function_node(0);

    test_expr_evaluate_function_1(0);
    test_expr_evaluate_function_2(0);
    test_expr_evaluate_function_3(0);

    test_expr_evaluate_1(0);

    test_expr_assign_values_variables(0);

    test_expr_inverse(0);

    test_expr_project(0);
    test_expr_reject(0);

    test_decode_function_infix_expr(0);
    test_decode_function_prefix_expr(0);
    test_decode_function_json_expr(0);

    test_expr_clone_tree(0);

    test_expr_remove_redundant_nodes_1(0);

    test_expr_exp_mv3_1(0);
    test_expr_exp_mv3_2(0);
    test_expr_exp_mv3_3(0);
    test_expr_exp_mv3_4(0);
    test_expr_exp_mv3_5(0);
    test_expr_exp_mv3_6(0);
    test_expr_exp_mv3_7(1);

    test_expr_exp_3d_1(0);
    test_expr_exp_3d_2(1);
    test_expr_exp_3d_3(0);
    test_expr_exp_3d_4(0);
    test_expr_exp_3d_5(0);

    u64 unverified_test_count = enabled_test_count - passed_test_count - failed_test_count;
    printf("Available tests: %lu\n", available_test_count);
    printf("Enabled tests: %lu\n", enabled_test_count);
    printf("Passed tests: %lu\n", passed_test_count);
    printf("Failed tests: %lu\n", failed_test_count);
    printf("Unverified tests: %lu\n", unverified_test_count);

    runtime_finalize();

    return 0;
}
#endif
