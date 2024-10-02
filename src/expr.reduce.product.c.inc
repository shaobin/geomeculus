/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
static u64 bubble_sort_dim_indexes(u64* dim_indexes, u64 num_dim_indexes)
{
    u64 swap_count = 0;
    for (u64 i = 0; i < num_dim_indexes; ++i)
    {
        for (u64 j = 0; j < num_dim_indexes - i - 1; ++j)
        {
            if (dim_indexes[j] > dim_indexes[j + 1])
            {
                u64 tmp = dim_indexes[j];
                dim_indexes[j] = dim_indexes[j + 1];
                dim_indexes[j + 1] = tmp;
                swap_count++;
            }
        }
    }
    return swap_count;
}

static u64 contains_dim_index(u64* dim_indexes, u64 num_dim_indexes, u64 dim_index)
{
    for (u64 i = 0; i < num_dim_indexes; ++i)
    {
        if (dim_indexes[i] == dim_index)
        {
            return 1;
        }
    }
    return 0;
}

/*
 * This function reduces a product of basis vectors and scalars.
 * The scalars include scalar constants, scalar variables, and scalar-valued functions.
 * For scalar-valued functions, they can be evaluated into scalar constants or scalar variables;
 * otherwise, we cannot reduce the product.
 */
static void* expr_reduce_product_node(void* product_node)
{
    struct variadic_node* prod_node = product_node;
    struct variadic_node* var_node = expr_create_geometric_product_node();

    struct expr_node* child = prod_node->head;
    for (u64 i = 0; i < prod_node->length; ++i)
    {
        struct expr_node* reduced_child = expr_reduce(child);
        expr_variadic_node_append_child(var_node, reduced_child);
        child = child->next;
    }

    // step 1: create containers for the basis vectors, scalars, and scalar-valued functions in the product
    u64* dim_indexes = x_malloc(var_node->length * sizeof(u64));
    u64 basis_vec_count = 0;

    f64 scalar = 1.0;

    u64* scalar_variables = x_malloc(var_node->length * sizeof(u64));
    u64 scalar_variable_count = 0;

    void** scalar_funcs = x_malloc(var_node->length * sizeof(void*));
    u64 scalar_func_count = 0;

    u64 non_scalar_func_count = 0;

    // step 2: populate the containers
    child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        if (expr_is_basis_vector_node(child))
        {
            dim_indexes[basis_vec_count++] = ((struct leaf_node*)child)->dim_index;
        }
        else if (expr_is_instantiated_scalar_node(child))
        {
            scalar *= ((struct leaf_node*)child)->f64_value;
        }
        else if (expr_is_scalar_variable_node(child))
        {
            scalar_variables[scalar_variable_count++] = ((struct leaf_node*)child)->symbol->u64_value;
        }
        else if (expr_is_scalar_function_node(child))
        {
            scalar_funcs[scalar_func_count++] = expr_clone_tree(child);
        }
        else if (expr_is_function_node(child))
        {
            non_scalar_func_count++;
        }
        else
        {
            printf("Error: Unknown node type %lu\n", ((struct expr_node*)child)->type);
        }

        child = child->next;
    }

    if (f64_is_zero(scalar))
    {
        x_free(dim_indexes);
        x_free(scalar_variables);

        for (u64 k = 0; k < scalar_func_count; ++k)
        {
            expr_free_tree(scalar_funcs[k]);
        }
        x_free(scalar_funcs);

        expr_free_tree(var_node);

        return expr_create_scalar_node_with_f64(scalar);
    }

    if (non_scalar_func_count)
    {
        x_free(dim_indexes);
        x_free(scalar_variables);

        for (u64 k = 0; k < scalar_func_count; ++k)
        {
            expr_free_tree(scalar_funcs[k]);
        }
        x_free(scalar_funcs);

        return var_node;
    }

    u64 swap_count = bubble_sort_dim_indexes(dim_indexes, basis_vec_count);
    if (swap_count % 2 == 1)
    {
        scalar *= -1.0;
    }

    if (scalar_variable_count == 0 && scalar_func_count == 0 && f64_is_close(scalar, 0.0))
    {
        x_free(dim_indexes);
        x_free(scalar_variables);

        for (u64 k = 0; k < scalar_func_count; ++k)
        {
            expr_free_tree(scalar_funcs[k]);
        }
        x_free(scalar_funcs);

        expr_free_tree(var_node);
        return expr_create_scalar_node_with_f64(scalar);
    }

    u64* basis_vectors = basis_vec_count ? x_malloc(basis_vec_count * sizeof(u64)) : NULL;
    u64* basis_vector2counts = basis_vec_count ? x_malloc(basis_vec_count * sizeof(u64)) : NULL;
    u64 unique_basis_vector_count = 0;
    for (u64 i = 0; i < basis_vec_count; ++i)
    {
        if (contains_dim_index(basis_vectors, unique_basis_vector_count, dim_indexes[i]))
        {
            basis_vector2counts[unique_basis_vector_count - 1]++;
        }
        else
        {
            basis_vectors[unique_basis_vector_count] = dim_indexes[i];
            basis_vector2counts[unique_basis_vector_count++] = 1;
        }
    }

    expr_free_tree(var_node);
    var_node = expr_create_geometric_product_node();

    if (algebra_ctx)
    {
        for (i64 i = (i64)unique_basis_vector_count - 1; i >= 0; --i)
        {
            i32 sig = algebra_ctx_dim_signature(basis_vectors[i]);
            if (sig == 1)
            {
                if (basis_vector2counts[i] % 2 == 1)
                {
                    expr_variadic_node_prepend_child(var_node, expr_create_basis_vector_node(basis_vectors[i]));
                }
            }
            else if (sig == -1)
            {
                u64 count_remainder = basis_vector2counts[i] % 4;
                if (count_remainder == 1)
                {
                    expr_variadic_node_prepend_child(var_node, expr_create_basis_vector_node(basis_vectors[i]));
                }
                else if (count_remainder == 2)
                {
                    scalar *= -1.0;
                }
                else if (count_remainder == 3)
                {
                    scalar *= -1.0;
                    expr_variadic_node_prepend_child(var_node, expr_create_basis_vector_node(basis_vectors[i]));
                }
            }
            else if (sig == 0)
            {
                if (basis_vector2counts[i] > 1)
                {
                    scalar = 0.0;
                }
                else
                {
                    expr_variadic_node_prepend_child(var_node, expr_create_basis_vector_node(basis_vectors[i]));
                }
            }
            else
            {
                printf("Error: Unknown signature %d\n", sig);
            }
        }
    }
    else
    {
        for (i64 i = (i64)unique_basis_vector_count - 1; i >= 0; --i)
        {
            if (basis_vector2counts[i] % 2 == 1)
            {
                expr_variadic_node_prepend_child(var_node, expr_create_basis_vector_node(basis_vectors[i]));
            }
        }
    }

    for (u64 i = 0; i < scalar_variable_count; ++i)
    {
        expr_variadic_node_append_child(var_node, expr_create_scalar_variable_node(scalar_variables[i]));
    }

    for (u64 i = 0; i < scalar_func_count; ++i)
    {
        struct variadic_node* func_node = scalar_funcs[i];
        expr_variadic_node_append_child(var_node, func_node);
    }

    if (!f64_is_close(scalar, 1.0))
    {
        expr_variadic_node_prepend_child(var_node, expr_create_scalar_node_with_f64(scalar));
    }

    if (var_node->length == 0)
    {
        if (basis_vec_count)
        {
            x_free(basis_vectors);
            x_free(basis_vector2counts);
        }
        x_free(dim_indexes);
        x_free(scalar_variables);
        x_free(scalar_funcs);
        x_free(var_node);
        return expr_create_scalar_node_with_f64(1.0);
    }

    if (var_node->length == 1)
    {
        struct expr_node* child = var_node->head;

        if (basis_vec_count)
        {
            x_free(basis_vectors);
            x_free(basis_vector2counts);
        }

        x_free(dim_indexes);
        x_free(scalar_variables);
        x_free(scalar_funcs);
        x_free(var_node);
        return child;
    }

    if (basis_vec_count)
    {
        x_free(basis_vectors);
        x_free(basis_vector2counts);
    }

    x_free(dim_indexes);
    x_free(scalar_variables);
    x_free(scalar_funcs);

    return var_node;
}
