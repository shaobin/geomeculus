/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <math.h>
#include <string.h>

#include "func_repo.h"
#include "mm.h"
#include "expr.h"
#include "string_util.h"
#include "special_func.h"
#include "constants.h"

u64 const FUNC_FLAG_MASK = 0xffffffff00000000;
u64 const FUNC_ID_MASK =   0x00000000ffffffff;

u64 const FUNC_FLAG_NONE =   0x0;
u64 const FUNC_FLAG_SCALAR = 0x1;

struct func_desc
{
    char* name;
    union
    {
        struct
        {
            u32 id;
            u32 flags;
        };
        u64 signature;
    };

    expr_func func;
    expr_func derivative;
    char const* notes;
};

#define CONSTANT_FUNC(name, value)                                                   \
static void* const_##name()                                                           \
{                                                                                    \
    return expr_create_scalar_node_with_f64(value);                                  \
}

#define SINGLE_ARG_SCALAR_FUNC(name)                                                 \
static void* func_##name(void* func)                                                 \
{                                                                                    \
    struct variadic_node* var_node = (struct variadic_node*)func;                    \
    if (var_node->length == 1)                                                       \
    {                                                                                \
        void* child = var_node->head;                                                \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            struct leaf_node* leaf_node = (struct leaf_node*)child;                  \
            return expr_create_scalar_node_with_f64(name(leaf_node->f64_value));     \
        }                                                                            \
    }                                                                                \
    return expr_clone_tree(func);                                                    \
}

#define DOUBLE_ARG_SCALAR_FUNC(name)                                                 \
static void* func_##name(void* func)                                                 \
{                                                                                    \
    struct variadic_node* var_node = (struct variadic_node*)func;                    \
    if (var_node->length == 2)                                                       \
    {                                                                                \
        f64 arg1 = 0.0;                                                              \
        f64 arg2 = 0.0;                                                              \
        struct leaf_node* child = var_node->head;                                    \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            arg1 = child->f64_value;                                                 \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            return expr_clone_tree(func);                                            \
        }                                                                            \
        child = child->next;                                                         \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            arg2 = child->f64_value;                                                 \
        }                                                                            \
        else                                                                         \
        {                                                                            \
            return expr_clone_tree(func);                                            \
        }                                                                            \
        return expr_create_scalar_node_with_f64(name(arg1, arg2));                   \
    }                                                                                \
    return expr_clone_tree(func);                                                    \
}

#define TRIPLE_ARG_SCALAR_FUNC(name)                                                 \
static void* func_##name(void* func)                                                 \
{                                                                                    \
    struct variadic_node* var_node = (struct variadic_node*)func;                    \
    if (var_node->length == 3)                                                       \
    {                                                                                \
        f64 arg1 = 0.0;                                                              \
        f64 arg2 = 0.0;                                                              \
        f64 arg3 = 0.0;                                                              \
        struct leaf_node* child = var_node->head;                                    \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            arg1 = child->f64_value;                                                 \
        }                                                                            \
        child = child->next;                                                         \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            arg2 = child->f64_value;                                                 \
        }                                                                            \
        child = child->next;                                                         \
        if (expr_is_instantiated_scalar_node(child))                                 \
        {                                                                            \
            arg3 = child->f64_value;                                                 \
        }                                                                            \
        return expr_create_scalar_node_with_f64(name(arg1, arg2, arg3));             \
    }                                                                                \
    return expr_clone_tree(func);                                                    \
}

CONSTANT_FUNC(pi, M_PI)
CONSTANT_FUNC(e, M_E)

SINGLE_ARG_SCALAR_FUNC(fabs)
SINGLE_ARG_SCALAR_FUNC(exp2)
SINGLE_ARG_SCALAR_FUNC(expm1)
SINGLE_ARG_SCALAR_FUNC(sqrt)
SINGLE_ARG_SCALAR_FUNC(sin)
SINGLE_ARG_SCALAR_FUNC(cos)
SINGLE_ARG_SCALAR_FUNC(tan)
SINGLE_ARG_SCALAR_FUNC(asin)
SINGLE_ARG_SCALAR_FUNC(acos)
SINGLE_ARG_SCALAR_FUNC(atan)
SINGLE_ARG_SCALAR_FUNC(sinh)
SINGLE_ARG_SCALAR_FUNC(cosh)
SINGLE_ARG_SCALAR_FUNC(tanh)
SINGLE_ARG_SCALAR_FUNC(asinh)
SINGLE_ARG_SCALAR_FUNC(acosh)
SINGLE_ARG_SCALAR_FUNC(atanh)
SINGLE_ARG_SCALAR_FUNC(erf)
SINGLE_ARG_SCALAR_FUNC(erfc)
SINGLE_ARG_SCALAR_FUNC(tgamma)
SINGLE_ARG_SCALAR_FUNC(lgamma)
SINGLE_ARG_SCALAR_FUNC(ceil)
SINGLE_ARG_SCALAR_FUNC(floor)
SINGLE_ARG_SCALAR_FUNC(trunc)
SINGLE_ARG_SCALAR_FUNC(round)
SINGLE_ARG_SCALAR_FUNC(nearbyint)
SINGLE_ARG_SCALAR_FUNC(rint)
SINGLE_ARG_SCALAR_FUNC(cbrt)
SINGLE_ARG_SCALAR_FUNC(logb)
SINGLE_ARG_SCALAR_FUNC(log2)
SINGLE_ARG_SCALAR_FUNC(log10)
SINGLE_ARG_SCALAR_FUNC(log1p)

SINGLE_ARG_SCALAR_FUNC(factorial)
SINGLE_ARG_SCALAR_FUNC(double_factorial)

DOUBLE_ARG_SCALAR_FUNC(fmod)
DOUBLE_ARG_SCALAR_FUNC(remainder)
DOUBLE_ARG_SCALAR_FUNC(fmax)
DOUBLE_ARG_SCALAR_FUNC(fmin)
DOUBLE_ARG_SCALAR_FUNC(fdim)
DOUBLE_ARG_SCALAR_FUNC(hypot)
DOUBLE_ARG_SCALAR_FUNC(nextafter)
DOUBLE_ARG_SCALAR_FUNC(copysign)

DOUBLE_ARG_SCALAR_FUNC(boys)

TRIPLE_ARG_SCALAR_FUNC(fma)

static void* func_null()
{
    return NULL;
}

static void* func_pow(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 2)
    {
        struct expr_node* base = var_node->head;
        struct expr_node* exponent = base->next;

        if (expr_is_instantiated_scalar_node(base) && expr_is_instantiated_scalar_node(exponent))
        {
            struct leaf_node* base_leaf = (struct leaf_node*)base;
            struct leaf_node* exponent_leaf = (struct leaf_node*)exponent;
            return expr_create_scalar_node_with_f64(pow(base_leaf->f64_value, exponent_leaf->f64_value));
        }

        void* ln_b = expr_create_function_node("log");
        expr_variadic_node_append_child(ln_b, expr_clone_tree(base));

        void* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_clone_tree(exponent));
        expr_variadic_node_append_child(prod, ln_b);

        void* result = expr_create_function_node("exp");
        expr_variadic_node_append_child(result, prod);

        return result;
    }

    return expr_clone_tree(func);
}

static void* func_exp(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* child = var_node->head;
        if (expr_is_instantiated_scalar_node(child))
        {
            struct leaf_node* leaf_node = child;
            return expr_create_scalar_node_with_f64(exp(leaf_node->f64_value));
        }

        if (expr_is_function_node(child))
        {
            struct variadic_node* func_node = child;
            if (strcmp(func_node->symbol->name, "log") == 0)
            {
                return expr_clone_tree(func_node->head);
            }
        }
    }
    return expr_clone_tree(func);
}

static void* func_log(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* child = var_node->head;
        if (expr_is_instantiated_scalar_node(child))
        {
            struct leaf_node* leaf_node = (struct leaf_node*)child;

            f64 value = leaf_node->f64_value;

            if (value > 0)
            {
                return expr_create_scalar_node_with_f64(log(value));
            }
        }
        else if (expr_is_function_node(child))
        {
            struct variadic_node* func_node = child;
            if (strcmp(func_node->symbol->name, "exp") == 0)
            {
                return expr_clone_tree(func_node->head);
            }
        }
    }
    return expr_clone_tree(func);
}

static void* func_negative(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* child = var_node->head;
        if (expr_is_instantiated_scalar_node(child))
        {
            struct leaf_node* leaf_node = (struct leaf_node*)child;
            return expr_create_scalar_node_with_f64(-(leaf_node->f64_value));
        }

        void* minus_one = expr_create_scalar_node_with_f64(-1.0);
        void* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, minus_one);
        expr_variadic_node_append_child(prod, expr_clone_tree(child));

        return prod;
    }
    return expr_clone_tree(func);
}

static void* func_reciprocal(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* child = var_node->head;
        if (expr_is_instantiated_scalar_node(child))
        {
            struct leaf_node* leaf_node = (struct leaf_node*)child;
            return expr_create_scalar_node_with_f64(1.0 / leaf_node->f64_value);
        }
    }
    return expr_clone_tree(func);
}

static void* func_cot(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* tan_x = expr_create_function_node("tan");
        expr_variadic_node_append_child(tan_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_tan_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_tan_x, tan_x);

        return one_over_tan_x;
    }
    return expr_clone_tree(func);
}

static void* func_sec(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* cos_x = expr_create_function_node("cos");
        expr_variadic_node_append_child(cos_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_cos_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_cos_x, cos_x);

        return one_over_cos_x;
    }
    return expr_clone_tree(func);
}

static void* func_csc(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* sin_x = expr_create_function_node("sin");
        expr_variadic_node_append_child(sin_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_sin_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_sin_x, sin_x);

        return one_over_sin_x;
    }
    return expr_clone_tree(func);
}

static void* func_acot(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* atan_x = expr_create_function_node("atan");
        expr_variadic_node_append_child(atan_x, expr_clone_tree(var_node->head));

        struct variadic_node* minus_atan_x = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_atan_x, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_atan_x, atan_x);

        struct variadic_node* pi_over_two_minus_atan_x = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(pi_over_two_minus_atan_x, expr_create_scalar_node_with_f64(M_PI_2));
        expr_variadic_node_append_child(pi_over_two_minus_atan_x, minus_atan_x);

        return pi_over_two_minus_atan_x;
    }
    return expr_clone_tree(func);
}

static void* func_asec(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* acos_one_over_x = expr_create_function_node("acos");
        expr_variadic_node_append_child(acos_one_over_x, one_over_x);

        return acos_one_over_x;
    }
    return expr_clone_tree(func);
}

static void* func_acsc(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* asin_one_over_x = expr_create_function_node("asin");
        expr_variadic_node_append_child(asin_one_over_x, one_over_x);

        return asin_one_over_x;
    }
    return expr_clone_tree(func);
}

static void* func_coth(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* tanh_x = expr_create_function_node("tanh");
        expr_variadic_node_append_child(tanh_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_tanh_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_tanh_x, tanh_x);

        return one_over_tanh_x;
    }
    return expr_clone_tree(func);
}

static void* func_sech(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* cosh_x = expr_create_function_node("cosh");
        expr_variadic_node_append_child(cosh_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_cosh_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_cosh_x, cosh_x);

        return one_over_cosh_x;
    }
    return expr_clone_tree(func);
}

static void* func_csch(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        void* sinh_x = expr_create_function_node("sinh");
        expr_variadic_node_append_child(sinh_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_sinh_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_sinh_x, sinh_x);

        return one_over_sinh_x;
    }
    return expr_clone_tree(func);
}

static void* func_acoth(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* atanh_one_over_x = expr_create_function_node("atanh");
        expr_variadic_node_append_child(atanh_one_over_x, one_over_x);

        return atanh_one_over_x;
    }
    return expr_clone_tree(func);
}

static void* func_asech(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* acosh_one_over_x = expr_create_function_node("acosh");
        expr_variadic_node_append_child(acosh_one_over_x, one_over_x);

        return acosh_one_over_x;
    }
    return expr_clone_tree(func);
}

static void* func_acsch(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* asinh_one_over_x = expr_create_function_node("asinh");
        expr_variadic_node_append_child(asinh_one_over_x, one_over_x);

        return asinh_one_over_x;
    }
    return expr_clone_tree(func);
}

static void* func_derivative_constant(void* func)
{
    return expr_create_scalar_node_with_f64(0.0);
}

static void* func_derivative_negative(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_create_scalar_node_with_f64(-1.0);
    }
    return NULL;
}

static void* func_derivative_exp(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        return expr_clone_tree(var_node);
    }
    return NULL;
}

static void* func_derivative_log(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, expr_clone_tree(var_node->head));
        return d_expr;
    }
    return NULL;
}

static void* func_derivative_reciprocal(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        struct variadic_node* arg_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(arg_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(arg_squared, expr_clone_tree(var_node->head));

        struct variadic_node* arg_squared_reciprocal = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(arg_squared_reciprocal, arg_squared);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(d_expr, arg_squared_reciprocal);
        return d_expr;
    }
    return NULL;
}

static void* func_derivative_sqrt(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        struct variadic_node* sqrt_arg = expr_clone_tree(func);
        struct variadic_node* sqrt_arg_reciprocal = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(sqrt_arg_reciprocal, sqrt_arg);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(0.5));
        expr_variadic_node_append_child(d_expr, sqrt_arg_reciprocal);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_sin(void* func)
{
    struct variadic_node* var_node = func;
    if (var_node->length == 1)
    {
        struct variadic_node* d_expr = expr_create_function_node("cos");
        expr_variadic_node_append_child(d_expr, expr_clone_tree(var_node->head));
        return d_expr;
    }
    return NULL;
}

static void* func_derivative_cos(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* d_expr = expr_create_function_node("sin");
        expr_variadic_node_append_child(d_expr, expr_clone_tree(var_node->head));
        struct variadic_node* minus_d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_d_expr, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_d_expr, d_expr);
        return minus_d_expr;
    }
    return NULL;
}

static void* func_derivative_tan(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* sec_x = expr_create_function_node("sec");
        expr_variadic_node_append_child(sec_x, expr_clone_tree(var_node->head));

        struct variadic_node* sec_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(sec_x_squared, sec_x);
        expr_variadic_node_append_child(sec_x_squared,expr_clone_tree(sec_x));

        return sec_x_squared;
    }
    return NULL;
}

static void* func_derivative_cot(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* csc_x = expr_create_function_node("csc");
        expr_variadic_node_append_child(csc_x, expr_clone_tree(var_node->head));

        struct variadic_node* csc_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(csc_x_squared, csc_x);
        expr_variadic_node_append_child(csc_x_squared, expr_clone_tree(csc_x));

        struct variadic_node* minus_csc_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_csc_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_csc_x_squared, csc_x_squared);

        return minus_csc_x_squared;
    }
    return NULL;
}

static void* func_derivative_sec(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* sec_x = expr_create_function_node("sec");
        expr_variadic_node_append_child(sec_x, expr_clone_tree(var_node->head));

        struct variadic_node* tan_x = expr_create_function_node("tan");
        expr_variadic_node_append_child(tan_x, expr_clone_tree(var_node->head));

        struct variadic_node* sec_x_tan_x = expr_create_geometric_product_node();
        expr_variadic_node_append_child(sec_x_tan_x, sec_x);
        expr_variadic_node_append_child(sec_x_tan_x, tan_x);

        return sec_x_tan_x;
    }
    return NULL;
}

static void* func_derivative_csc(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* csc_x = expr_create_function_node("csc");
        expr_variadic_node_append_child(csc_x, expr_clone_tree(var_node->head));

        struct variadic_node* cot_x = expr_create_function_node("cot");
        expr_variadic_node_append_child(cot_x, expr_clone_tree(var_node->head));

        struct variadic_node* minus_csc_x_cot_x = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_csc_x_cot_x, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_csc_x_cot_x, csc_x);
        expr_variadic_node_append_child(minus_csc_x_cot_x, cot_x);

        return minus_csc_x_cot_x;
    }
    return NULL;
}

static void* func_derivative_asin(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_minus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_minus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_minus_x_squared, minus_x_squared);

        struct variadic_node* sqrt_one_minus_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_minus_x_squared, one_minus_x_squared);

        struct variadic_node* one_over_sqrt_one_minus_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_sqrt_one_minus_x_squared, sqrt_one_minus_x_squared);

        return one_over_sqrt_one_minus_x_squared;
    }
    return NULL;
}

static void* func_derivative_acos(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_minus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_minus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_minus_x_squared, minus_x_squared);

        struct variadic_node* sqrt_one_minus_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_minus_x_squared, one_minus_x_squared);

        struct variadic_node* one_over_sqrt_one_minus_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_sqrt_one_minus_x_squared, sqrt_one_minus_x_squared);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(d_expr, one_over_sqrt_one_minus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_atan(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_plus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_x_squared, x_squared);

        struct variadic_node* one_over_one_plus_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_one_plus_x_squared, one_plus_x_squared);

        return one_over_one_plus_x_squared;
    }
    return NULL;
}

static void* func_derivative_acot(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_plus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_x_squared, x_squared);

        struct variadic_node* one_over_one_plus_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_one_plus_x_squared, one_plus_x_squared);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(d_expr, one_over_one_plus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_asec(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x_squared, x_squared);

        struct variadic_node* minus_one_over_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_one_over_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_one_over_x_squared, one_over_x_squared);

        struct variadic_node* one_plus_minus_one_over_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_minus_one_over_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_minus_one_over_x_squared, minus_one_over_x_squared);

        struct variadic_node* sqrt_one_plus_minus_one_over_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_plus_minus_one_over_x_squared, one_plus_minus_one_over_x_squared);

        struct variadic_node* prod_node = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod_node, sqrt_one_plus_minus_one_over_x_squared);
        expr_variadic_node_append_child(prod_node, expr_clone_tree(x_squared));

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, prod_node);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_acsc(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x_squared, x_squared);

        struct variadic_node* minus_one_over_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_one_over_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_one_over_x_squared, one_over_x_squared);

        struct variadic_node* one_plus_minus_one_over_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_minus_one_over_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_minus_one_over_x_squared, minus_one_over_x_squared);

        struct variadic_node* sqrt_one_plus_minus_one_over_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_plus_minus_one_over_x_squared, one_plus_minus_one_over_x_squared);

        struct variadic_node* prod_node = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod_node, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(prod_node, sqrt_one_plus_minus_one_over_x_squared);
        expr_variadic_node_append_child(prod_node, expr_clone_tree(x_squared));

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, prod_node);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_sinh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* d_expr = expr_create_function_node("cosh");
        expr_variadic_node_append_child(d_expr, expr_clone_tree(var_node->head));

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_cosh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* d_expr = expr_create_function_node("sinh");
        expr_variadic_node_append_child(d_expr, expr_clone_tree(var_node->head));

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_tanh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* sech_x = expr_create_function_node("sech");
        expr_variadic_node_append_child(sech_x, expr_clone_tree(var_node->head));

        struct variadic_node* sech_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(sech_x_squared, sech_x);
        expr_variadic_node_append_child(sech_x_squared, expr_clone_tree(sech_x));

        return sech_x_squared;
    }
    return NULL;
}

static void* func_derivative_coth(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* csch_x = expr_create_function_node("csch");
        expr_variadic_node_append_child(csch_x, expr_clone_tree(var_node->head));

        struct variadic_node* csch_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(csch_x_squared, csch_x);
        expr_variadic_node_append_child(csch_x_squared, expr_clone_tree(csch_x));

        struct variadic_node* minus_csch_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_csch_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_csch_x_squared, csch_x_squared);

        return minus_csch_x_squared;
    }
    return NULL;
}

static void* func_derivative_sech(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* sech_x = expr_create_function_node("sech");
        expr_variadic_node_append_child(sech_x, expr_clone_tree(var_node->head));

        struct variadic_node* tanh_x = expr_create_function_node("tanh");
        expr_variadic_node_append_child(tanh_x, expr_clone_tree(var_node->head));

        struct variadic_node* sech_x_tanh_x = expr_create_geometric_product_node();
        expr_variadic_node_append_child(sech_x_tanh_x, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(sech_x_tanh_x, sech_x);
        expr_variadic_node_append_child(sech_x_tanh_x, tanh_x);

        return sech_x_tanh_x;
    }
    return NULL;
}

static void* func_derivative_csch(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* csch_x = expr_create_function_node("csch");
        expr_variadic_node_append_child(csch_x, expr_clone_tree(var_node->head));

        struct variadic_node* coth_x = expr_create_function_node("coth");
        expr_variadic_node_append_child(coth_x, expr_clone_tree(var_node->head));

        struct variadic_node* minus_csch_x_coth_x = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_csch_x_coth_x, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_csch_x_coth_x, csch_x);
        expr_variadic_node_append_child(minus_csch_x_coth_x, coth_x);

        return minus_csch_x_coth_x;
    }
    return NULL;
}

static void* func_derivative_asinh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_plus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_x_squared, x_squared);

        struct variadic_node* sqrt_one_plus_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_plus_x_squared, one_plus_x_squared);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, sqrt_one_plus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_acosh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* x_squared_minus_one = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(x_squared_minus_one, x_squared);
        expr_variadic_node_append_child(x_squared_minus_one, expr_create_scalar_node_with_f64(-1.0));

        struct variadic_node* sqrt_x_squared_minus_one = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_x_squared_minus_one, x_squared_minus_one);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, sqrt_x_squared_minus_one);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_atanh(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_minus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_minus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_minus_x_squared, minus_x_squared);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, one_minus_x_squared);

        return d_expr;

    }
    return NULL;
}

static void* func_derivative_acoth(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_minus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_minus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_minus_x_squared, minus_x_squared);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, one_minus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_asech(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_minus_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_minus_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_minus_x_squared, minus_x_squared);

        struct variadic_node* sqrt_one_minus_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_minus_x_squared, one_minus_x_squared);

        struct variadic_node* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(prod, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(prod, sqrt_one_minus_x_squared);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, prod);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_acsch(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_x_squared = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x_squared, x_squared);

        struct variadic_node* one_plus_one_over_x_squared = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_one_over_x_squared, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_one_over_x_squared, one_over_x_squared);

        struct variadic_node* sqrt_one_plus_one_over_x_squared = expr_create_function_node("sqrt");
        expr_variadic_node_append_child(sqrt_one_plus_one_over_x_squared, one_plus_one_over_x_squared);

        struct variadic_node* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(prod, expr_clone_tree(x_squared));
        expr_variadic_node_append_child(prod, sqrt_one_plus_one_over_x_squared);

        struct variadic_node* d_expr = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(d_expr, prod);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_erf(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* exp_minus_x_squared = expr_create_function_node("exp");
        expr_variadic_node_append_child(exp_minus_x_squared, minus_x_squared);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(M_2_SQRTPI));
        expr_variadic_node_append_child(d_expr, exp_minus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_erfc(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* minus_x_squared = expr_create_geometric_product_node();
        expr_variadic_node_append_child(minus_x_squared, expr_create_scalar_node_with_f64(-1.0));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));
        expr_variadic_node_append_child(minus_x_squared, expr_clone_tree(var_node->head));

        struct variadic_node* exp_minus_x_squared = expr_create_function_node("exp");
        expr_variadic_node_append_child(exp_minus_x_squared, minus_x_squared);

        struct variadic_node* d_expr = expr_create_geometric_product_node();
        expr_variadic_node_append_child(d_expr, expr_create_scalar_node_with_f64(-M_2_SQRTPI));
        expr_variadic_node_append_child(d_expr, exp_minus_x_squared);

        return d_expr;
    }
    return NULL;
}

static void* func_derivative_log2(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(ONE_OVER_LN2));
        expr_variadic_node_append_child(prod, one_over_x);

        return prod;
    }
    return NULL;
}

static void* func_derivative_log10(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_over_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_x, expr_clone_tree(var_node->head));

        struct variadic_node* prod = expr_create_geometric_product_node();
        expr_variadic_node_append_child(prod, expr_create_scalar_node_with_f64(ONE_OVER_LN10));
        expr_variadic_node_append_child(prod, one_over_x);

        return prod;
    }
    return NULL;
}

static void* func_derivative_log1p(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    if (var_node->length == 1)
    {
        struct variadic_node* one_plus_x = expr_create_geometric_sum_node();
        expr_variadic_node_append_child(one_plus_x, expr_create_scalar_node_with_f64(1.0));
        expr_variadic_node_append_child(one_plus_x, expr_clone_tree(var_node->head));

        struct variadic_node* one_over_one_plus_x = expr_create_function_node("reciprocal");
        expr_variadic_node_append_child(one_over_one_plus_x, one_plus_x);

        return one_over_one_plus_x;
    }
    return NULL;
}

static struct list expr_funcs = {.length = 0, .head = NULL, .tail = NULL};

void func_register(char const* name, expr_func func, expr_func derivative_func, u64 flags, char const* notes)
{
    struct func_desc* fd = x_malloc(sizeof(struct func_desc));
    fd->name = (char*)name;
    fd->id = expr_funcs.length;
    fd->flags = flags;
    fd->func = func;
    fd->derivative = derivative_func;
    fd->notes = notes;
    list_append(&expr_funcs, fd);
}

void func_register_builtin_routines()
{
    struct func_desc* fd = x_calloc(1, sizeof(struct func_desc));
    list_append(&expr_funcs, fd);

    func_register("null", func_null, NULL, FUNC_FLAG_NONE, "(): null function (no-op)");

    func_register("pi", const_pi, func_derivative_constant, FUNC_FLAG_SCALAR, "(): constant pi");
    func_register("e", const_e, func_derivative_constant, FUNC_FLAG_SCALAR, "(): constant e");

    func_register("fabs", func_fabs, NULL, FUNC_FLAG_SCALAR, "(f64 x): absolute value");
    func_register("exp", func_exp, func_derivative_exp, FUNC_FLAG_SCALAR, "(f64 x): exponential");
    func_register("exp2", func_exp2, NULL, FUNC_FLAG_SCALAR, "(f64 x): 2^x");
    func_register("expm1", func_expm1, NULL, FUNC_FLAG_SCALAR, "(f64 x): e^x - 1");
    func_register("log", func_log, func_derivative_log, FUNC_FLAG_SCALAR, "(f64 x): natural logarithm");
    func_register("sqrt", func_sqrt, func_derivative_sqrt, FUNC_FLAG_SCALAR, "(f64 x): square root");
    func_register("sin", func_sin, func_derivative_sin, FUNC_FLAG_SCALAR, "(f64 x): sine");
    func_register("cos", func_cos, func_derivative_cos, FUNC_FLAG_SCALAR, "(f64 x): cosine");
    func_register("tan", func_tan, func_derivative_tan, FUNC_FLAG_SCALAR, "(f64 x): tangent, x in radians");
    func_register("cot", func_cot, func_derivative_cot, FUNC_FLAG_SCALAR, "(f64 x): cotangent, x in radians");
    func_register("sec", func_sec, func_derivative_sec, FUNC_FLAG_SCALAR, "(f64 x): secant, x in radians");
    func_register("csc", func_csc, func_derivative_csc, FUNC_FLAG_SCALAR, "(f64 x): cosecant, x in radians");
    func_register("asin", func_asin, func_derivative_asin, FUNC_FLAG_SCALAR, "(f64 x): arcsine, result in radians");
    func_register("acos", func_acos, func_derivative_acos, FUNC_FLAG_SCALAR, "(f64 x): arccosine, result in radians");
    func_register("atan", func_atan, func_derivative_atan, FUNC_FLAG_SCALAR, "(f64 x): arctangent, result in radians");
    func_register("acot", func_acot, func_derivative_acot, FUNC_FLAG_SCALAR, "(f64 x): arccotangent, result in radians");
    func_register("asec", func_asec, func_derivative_asec, FUNC_FLAG_SCALAR, "(f64 x): arcsecant, result in radians");
    func_register("acsc", func_acsc, func_derivative_acsc, FUNC_FLAG_SCALAR, "(f64 x): arccosecant, result in radians");
    func_register("sinh", func_sinh, func_derivative_sinh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic sine");
    func_register("cosh", func_cosh, func_derivative_cosh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic cosine");
    func_register("tanh", func_tanh, func_derivative_tanh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic tangent");
    func_register("coth", func_coth, func_derivative_coth, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic cotangent");
    func_register("sech", func_sech, func_derivative_sech, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic secant");
    func_register("csch", func_csch, func_derivative_csch, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic cosecant");
    func_register("asinh", func_asinh, func_derivative_asinh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arcsine");
    func_register("acosh", func_acosh, func_derivative_acosh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arccosine");
    func_register("atanh", func_atanh, func_derivative_atanh, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arctangent");
    func_register("acoth", func_acoth, func_derivative_acoth, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arccotangent");
    func_register("asech", func_asech, func_derivative_asech, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arcsecant");
    func_register("acsch", func_acsch, func_derivative_acsch, FUNC_FLAG_SCALAR, "(f64 x): hyperbolic arccosecant");
    func_register("erf", func_erf, func_derivative_erf, FUNC_FLAG_SCALAR, "(f64 x): error function");
    func_register("erfc", func_erfc, func_derivative_erfc, FUNC_FLAG_SCALAR, "(f64 x): complementary error function");
    func_register("tgamma", func_tgamma, NULL, FUNC_FLAG_SCALAR, "(f64 x): Gamma function");
    func_register("lgamma", func_lgamma, NULL, FUNC_FLAG_SCALAR, "(f64 x): natural logarithm of absolute value of Gamma function");
    func_register("ceil", func_ceil, NULL, FUNC_FLAG_SCALAR, "(f64 x): smallest integer not less than x");
    func_register("floor", func_floor, NULL, FUNC_FLAG_SCALAR, "(f64 x): largest integer not greater than x");
    func_register("trunc", func_trunc, NULL, FUNC_FLAG_SCALAR, "(f64 x): integer nearest to x, but not larger in magnitude");
    func_register("round", func_round, NULL, FUNC_FLAG_SCALAR, "(f64 x): integer nearest to x, rounding halfway cases away from zero, regardless of the current rounding direction");
    func_register("nearbyint", func_nearbyint, NULL, FUNC_FLAG_SCALAR, "(f64 x): integer nearest to x, rounding halfway cases to even");
    func_register("rint", func_rint, NULL, FUNC_FLAG_SCALAR, "(f64 x): integer nearest to x, rounding halfway cases to even");
    func_register("cbrt", func_cbrt, NULL, FUNC_FLAG_SCALAR, "(f64 x): cube root");
    func_register("logb", func_logb, NULL, FUNC_FLAG_SCALAR, "(f64 x): extract exponet from internal f64 representation of x as a f64 value");
    func_register("log2", func_log2, func_derivative_log2, FUNC_FLAG_SCALAR, "(f64 x): base-2 logarithm of x");
    func_register("log10", func_log10, func_derivative_log10, FUNC_FLAG_SCALAR, "(f64 x): base-10 logarithm of x");
    func_register("log1p", func_log1p, func_derivative_log1p, FUNC_FLAG_SCALAR, "(f64 x): natural logarithm of 1 + x");

    func_register("factorial", func_factorial, NULL, FUNC_FLAG_SCALAR, "(f64 n) n!");
    func_register("double_factorial", func_double_factorial, NULL, FUNC_FLAG_SCALAR, "(f64 n) n!!, factorial with every second value skipped");

    func_register("negative", func_negative, func_derivative_negative, FUNC_FLAG_NONE, "(x): -x");
    func_register("reciprocal", func_reciprocal, func_derivative_reciprocal, FUNC_FLAG_SCALAR, "(x): 1 / x");

    func_register("pow", func_pow, NULL, FUNC_FLAG_NONE, "(f64 base, f64 exponent): base raised to the power of exponent");
    func_register("fmod", func_fmod, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): x-n*y, where n is x / y with its fractional part truncated");
    func_register("remainder", func_remainder, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): x-n*y, where n is x / y, rounded to the nearest integer");
    func_register("fmax", func_fmax, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): maximum of x and y");
    func_register("fmin", func_fmin, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): minimum of x and y");
    func_register("fdim", func_fdim, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): positive difference of x and y, max(x - y, 0)");
    func_register("hypot", func_hypot, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): hypotenuse of right-angled triangle, sqrt(x^2 + y^2)");
    func_register("nextafter", func_nextafter, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): next representable f64 value after x in the direction of y");
    func_register("copysign", func_copysign, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y): a value whose absolute value matches that of x, the sign matches that of y");

    func_register("boys", func_boys, NULL, FUNC_FLAG_SCALAR, "(f64 n, f64 x) Boys function of order n and argument x");

    func_register("fma", func_fma, NULL, FUNC_FLAG_SCALAR, "(f64 x, f64 y, f64 z): x*y + z");
};

void func_print_repo()
{
    printf("Registered functions:\n\n");
    for (u64 i = 1; i < expr_funcs.length; ++i)
    {
        struct func_desc* fd = list_get(&expr_funcs, i);
        printf("  %s", fd->name);
        if (fd->notes)
        {
            printf("%s", fd->notes);
        }
        printf("\n\n");
    }
}

u64 func_get_help(char const* name)
{
    for (u64 i = 1; i < expr_funcs.length; ++i)
    {
        struct func_desc* fd = list_get(&expr_funcs, i);
        if (strcmp(name, fd->name) == 0)
        {
            printf("%s%s\n", fd->name, fd->notes);
            return 1;
        }
    }
    return 0;
}

void func_get_names(struct list* func_names)
{
    for (u64 i = 1; i < expr_funcs.length; ++i)
    {
        struct func_desc* fd = list_get(&expr_funcs, i);
        list_append(func_names, fd->name);
    }
}

void func_free_repo()
{
    list_clear(&expr_funcs, 1);
}

char const* func_is_valid_name(char const* name)
{
    for (u64 i = 1; i < expr_funcs.length; ++i)
    {
        struct func_desc* fd = list_get(&expr_funcs, i);
        if (strcmp(name, fd->name) == 0)
        {
            return fd->name;
        }
    }
    return NULL;
}

u64 func_get_signature(char const* name)
{
    for (u64 i = 1; i < expr_funcs.length; ++i)
    {
        struct func_desc* fd = list_get(&expr_funcs, i);
        if (strcmp(name, fd->name) == 0)
        {
            return fd->signature;
        }
    }
    return 0;
}

u64 func_is_scalar(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    u64 func_id = (var_node->symbol->u64_value) & FUNC_ID_MASK;
    struct func_desc* fd = list_get(&expr_funcs, func_id);
    return fd->flags & FUNC_FLAG_SCALAR;
}

void* func_evaluate(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    u64 func_id = (var_node->symbol->u64_value) & FUNC_ID_MASK;
    if (func_id == 0)
    {
        return expr_clone_tree(func);
    }
    struct func_desc* fd = list_get(&expr_funcs, func_id);
    return fd->func(func);
}

void* func_derivative(void* func)
{
    struct variadic_node* var_node = (struct variadic_node*)func;
    u64 func_id = (var_node->symbol->u64_value) & FUNC_ID_MASK;
    if (func_id == 0)
    {
        return expr_clone_tree(func);
    }
    struct func_desc* fd = list_get(&expr_funcs, func_id);
    return fd->derivative(func);
}

