/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
struct scalar_factor
{
    f64 scalar;
    struct list* variable_ids;
    struct list* func_nodes;
};

void print_scalar_factors(struct list* sfs)
{
    struct list_node* outer_node = sfs->head; // outer list
    for (u64 i = 0; i < sfs->length; ++i)
    {
        struct scalar_factor* sf = outer_node->data;
        printf("scalar: %.5f\n", sf->scalar);
        if (sf->variable_ids != NULL)
        {
            struct list_node* inner_node = sf->variable_ids->head; // inner list
            printf("var_ids:");
            for (u64 j = 0; j < sf->variable_ids->length; ++j)
            {
                printf(" %lu", (u64)inner_node->data);
                inner_node = inner_node->next;
            }
            printf("\n");
        }
        if (sf->func_nodes != NULL)
        {
            struct list_node* inner_node = sf->func_nodes->head; // inner list
            for (u64 j = 0; j < sf->func_nodes->length; ++j)
            {
                expr_print_tree(inner_node->data);
                inner_node = inner_node->next;
            }
        }
        outer_node = outer_node->next;
    }
}

/*
  The node should be a leaf node, or product of scalar and sorted basis vectors
*/
static void* basis_blade_signature(struct expr_node* node, f64* coeff)
{
    if (expr_is_scalar_node(node))
    {
        *coeff = ((struct leaf_node*)node)->f64_value;
        struct bitmap* bitmap = bitmap_create(1);
        return bitmap;
    }

    if (expr_is_basis_vector_node(node))
    {
        struct leaf_node* bv = (struct leaf_node*)node;
        *coeff = 1.0;

        struct bitmap* bitmap = bitmap_create(bv->dim_index + 1);
        bitmap_set(bitmap, bv->dim_index);
        return bitmap;
    }

    *coeff = 1.0;

    if (expr_is_geometric_product_node(node))
    {
        struct variadic_node* gp = (struct variadic_node*)node;
        u64 max_dim_index = 0;
        struct expr_node* child = gp->head;
        for (u64 i = 0; i < gp->length; ++i)
        {
            if (expr_is_scalar_node(child))
            {
                *coeff = ((struct leaf_node*)child)->f64_value;
            }

            if (expr_is_basis_vector_node(child))
            {
                struct leaf_node* bv = (struct leaf_node*)child;
                if (bv->dim_index > max_dim_index)
                {
                    max_dim_index = bv->dim_index;
                }
            }

            child = child->next;
        }

        struct bitmap* bitmap = bitmap_create(max_dim_index + 1);

        child = gp->head;
        for (u64 i = 0; i < gp->length; ++i)
        {
            if (expr_is_basis_vector_node(child))
            {
                struct leaf_node* bv = (struct leaf_node*)child;
                bitmap_set(bitmap, bv->dim_index);
            }

            child = child->next;
        }

        return bitmap;
    }

    return NULL;
}

/*
  The node should be a leaf node, or product of scalars and variables
*/
static struct id_signature* variable_blade_signature(void* node, f64* coeff)
{
    struct id_signature* sig = id_signature_create();

    if (expr_is_instantiated_scalar_node(node))
    {
        *coeff = ((struct leaf_node*)node)->f64_value;
        return sig;
    }

    if (expr_is_scalar_variable_node(node))
    {
        struct leaf_node* v = (struct leaf_node*)node;
        *coeff = 1.0;
        id_signature_set(sig, v->symbol->u64_value);
        return sig;
    }

    *coeff = 1.0;
    struct variadic_node* var_node = (struct variadic_node*)node;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        if (expr_is_instantiated_scalar_node(child))
        {
            *coeff *= ((struct leaf_node*)child)->f64_value;
        }

        if (expr_is_scalar_variable_node(child))
        {
            struct leaf_node* v = (struct leaf_node*)child;
            id_signature_set(sig, v->symbol->u64_value);
        }

        child = child->next;
    }

    return sig;
}

static void callback_create_basis_vector(u64 dim_index, union b64 data)
{
    expr_variadic_node_append_child(data.pointer, expr_create_basis_vector_node(dim_index));
}

static void* create_blade_from_signature(void* bitmap)
{
    struct bitmap* signature = bitmap;
    u64 dim_count = bitmap_count_set_bits(signature);

    if (dim_count == 0)
    {
        return expr_create_scalar_node_with_f64(1.0);
    }

    if (dim_count == 1)
    {
        u64 dim_idx = bitmap_first_set_bit_index(signature);
        return expr_create_basis_vector_node(dim_idx);
    }

    void* var_node = expr_create_geometric_product_node();
    bitmap_iterate(signature, (union b64)var_node, callback_create_basis_vector);

    return var_node;
}

static void separate_instantiated_uninstantiated(void* expr, void** uninstantiated, void** instantiated)
{
    if (expr_is_leaf_node(expr))
    {
        if (expr_is_variable_node(expr))
        {
            *uninstantiated = expr_clone_tree(expr);
            *instantiated = expr_create_scalar_node_with_f64(1.0);
        }
        else
        {
            *uninstantiated = NULL;
            *instantiated = expr_clone_tree(expr);
        }
        return;
    }

    if (expr_is_function_node(expr))
    {
        *uninstantiated = expr_clone_tree(expr);
        *instantiated = expr_create_scalar_node_with_f64(1.0);
        return;
    }

    *uninstantiated = NULL;
    *instantiated = NULL;

    struct variadic_node* var_node = expr;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        void* _next = child->next;
        if (expr_is_instantiated_node(child))
        {
            if (*instantiated == NULL)
            {
                *instantiated = expr_create_geometric_product_node();
            }
            expr_variadic_node_append_child(*instantiated, expr_clone_tree(child));
        }
        else
        {
            if (*uninstantiated == NULL)
            {
                *uninstantiated = expr_create_geometric_product_node();
            }
            expr_variadic_node_append_child(*uninstantiated, expr_clone_tree(child));
        }
        child = _next;
    }

    if (*instantiated == NULL)
    {
        *instantiated = expr_create_scalar_node_with_f64(1.0);
    }
}

static void extract_scalar_factors(void* uninstantiated, struct scalar_factor* sf)
{
    if (uninstantiated == NULL)
    {
        return;
    }

    if (expr_is_scalar_variable_node(uninstantiated))
    {
        if (sf->variable_ids == NULL)
        {
            sf->variable_ids = list_create();
        }

        struct leaf_node* v_node = uninstantiated;
        list_prepend(sf->variable_ids, v_node->symbol->pointer);
        return;
    }

    if (expr_is_scalar_function_node(uninstantiated))
    {
        if (sf->func_nodes == NULL)
        {
            sf->func_nodes = list_create();
        }

        struct variadic_node* func_node = uninstantiated;
        list_prepend(sf->func_nodes, expr_clone_tree(func_node));
        return;
    }

    struct variadic_node* var_node = uninstantiated;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        extract_scalar_factors(child, sf);
        child = child->next;
    }
    return;
}

static void* create_scalar_terms_from_signature(struct id_signature* sig, f64 coeff)
{
    if (sig->length == 0)
    {
        return expr_create_scalar_node_with_f64(coeff);
    }

    if (f64_is_zero(coeff))
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    struct variadic_node* gp = expr_create_geometric_product_node();

    if (coeff != 1.0)
    {
        expr_variadic_node_prepend_child(gp, expr_create_scalar_node_with_f64(coeff));
    }

    struct id_count* id_count = sig->data;
    for (u64 i = 0; i < sig->length; ++i)
    {
        for (u64 j = 0; j < id_count->count; ++j)
        {
            struct leaf_node* v = expr_create_scalar_variable_node(id_count->id);
            expr_variadic_node_append_child(gp, v);
        }
        id_count = id_count->next;
    }

    if (gp->length == 1)
    {
        struct expr_node* child = gp->head;
        x_free(gp);
        return child;
    }

    return gp;
}


static void free_id_sig_to_coeff(struct b64_kv_array* id_sig_to_coeff)
{
    for (u64 i = 0; i < id_sig_to_coeff->length; ++i)
    {
        id_signature_free(id_sig_to_coeff->data[i].key.pointer);
    }
    b64_kv_array_free(id_sig_to_coeff, 0);
}

static u64 product_node_has_function_child(void* node)
{
    struct variadic_node* var_node = node;
    struct expr_node* child = var_node->head;
    for (u64 i = 0; i < var_node->length; ++i)
    {
        if (expr_is_function_node(child))
        {
            return 1;
        }
        child = child->next;
    }
    return 0;
}

/*
 * The input is a sum of product nodes.
 * Each product node is a product of scalars, variables, and scalar functions.
 */
static void* reduce_scalar_terms(void* node)
{
    struct variadic_node* var_node = node;
    struct b64_kv_array* sig2coeff = b64_kv_array_create(var_node->length);
    struct list* funcs_node = list_create();

    f64 scalar_sum = 0.0;
    struct expr_node* child = var_node->head;

    // each child is a product node
    for (u64 i = 0; i < var_node->length; ++i)
    {
        if (product_node_has_function_child(child))
        {
            list_append(funcs_node, child);
            child = child->next;
            continue;
        }

        f64 coeff;
        struct id_signature* sig = variable_blade_signature(child, &coeff);

        u64 ownership_transferred = 0;
        union b64 b64_coeff;
        if (b64_kv_array_get_ex(sig2coeff, sig, &b64_coeff, id_signature_compare))
        {
            b64_coeff.f64_value += coeff;
        }
        else
        {
            b64_coeff.f64_value = coeff;
        }
        b64_kv_array_set_ex(sig2coeff, sig, b64_coeff, id_signature_compare, &ownership_transferred);
        if(!ownership_transferred)
        {
            id_signature_free(sig);
        }

        child = child->next;
    }

    if (sig2coeff->length == 0)
    {
        if (funcs_node->length == 0)
        {
            free_id_sig_to_coeff(sig2coeff);
            list_free(funcs_node, 0);
            return expr_create_scalar_node_with_f64(scalar_sum);
        }
        else
        {
            struct variadic_node* sum_node = expr_create_geometric_sum_node();
            struct list_node* funcs_node_node = funcs_node->head;
            for (u64 i = 0; i < funcs_node->length; ++i)
            {
                expr_variadic_node_append_child(sum_node, expr_clone_tree(funcs_node_node->data));
                funcs_node_node = funcs_node_node->next;
            }
            free_id_sig_to_coeff(sig2coeff);
            list_free(funcs_node, 0);
            return sum_node;
        }

    }

    struct variadic_node* sum_node = expr_create_geometric_sum_node();
    struct b64_kv_pair* kv = sig2coeff->data;
    for (u64 i = 0; i < sig2coeff->length; ++i)
    {
        void* scalar_terms = create_scalar_terms_from_signature(kv->key.pointer, kv->value.f64_value);
        expr_variadic_node_append_child(sum_node, scalar_terms);
        kv++;
    }

    if (scalar_sum != 0.0)
    {
        expr_variadic_node_prepend_child(sum_node, expr_create_scalar_node_with_f64(scalar_sum));
    }

    if (funcs_node->length == 0)
    {
        list_free(funcs_node, 0);
    }
    else
    {
        struct list_node* funcs_node_node = funcs_node->head;
        for (u64 i = 0; i < funcs_node->length; ++i)
        {
            expr_variadic_node_append_child(sum_node, expr_clone_tree(funcs_node_node->data));
            funcs_node_node = funcs_node_node->next;
        }
        list_free(funcs_node, 0);
    }

    if (sum_node->length == 1)
    {
        struct expr_node* child = sum_node->head;
        x_free(sum_node);
        free_id_sig_to_coeff(sig2coeff);
        return child;
    }

    free_id_sig_to_coeff(sig2coeff);
    return sum_node;
}

static void* create_node_from_list_of_scalar_factors(struct list* list_scalar_factors)
{
    struct variadic_node* result_node = expr_create_geometric_sum_node();

    struct list_node* outer_node = list_scalar_factors->head; // outer list
    for (u64 i = 0; i < list_scalar_factors->length; ++i)
    {
        struct scalar_factor* sf = outer_node->data;
        struct list* variable_ids = sf->variable_ids;
        struct list* func_nodes = sf->func_nodes;

        if (f64_is_zero(sf->scalar))
        {
            outer_node = outer_node->next;
            continue;
        }

        void* gp = expr_create_geometric_product_node();

        if (!f64_is_close(sf->scalar, 1.0))
        {
            expr_variadic_node_prepend_child(gp, expr_create_scalar_node_with_f64(sf->scalar));
        }

        if (variable_ids != NULL)
        {
            struct list_node* variable_id_node = variable_ids->head; // inner list
            for (u64 j = 0; j < variable_ids->length; ++j)
            {
                union b64 var_id = {.pointer = variable_id_node->data};
                void* v = expr_create_scalar_variable_node(var_id.u64_value);
                expr_variadic_node_append_child(gp, v);
                variable_id_node = variable_id_node->next;
            }
        }

        if (func_nodes != NULL)
        {
            struct list_node* func_node = func_nodes->head; // inner list
            for (u64 j = 0; j < func_nodes->length; ++j)
            {
                struct variadic_node* f = expr_clone_tree(func_node->data);
                expr_variadic_node_append_child(gp, f);
                func_node = func_node->next;
            }
        }

        expr_variadic_node_append_child(result_node, gp);
        outer_node = outer_node->next;
    }

    void* reduced_result_node = reduce_scalar_terms(result_node);
    expr_free_tree(result_node);
    return reduced_result_node;
}

static void free_list_of_scalar_factors(struct list* scalar_factors)
{
    if (scalar_factors == NULL)
    {
        return;
    }

    struct list_node* outer_node = scalar_factors->head; // outer list

    for (u64 i = 0; i < scalar_factors->length; ++i)
    {
        struct scalar_factor* sf = outer_node->data;
        if (sf->variable_ids != NULL)
        {
            list_free(sf->variable_ids, 0);
        }
        if (sf->func_nodes != NULL)
        {
            struct list_node* inner_node = sf->func_nodes->head; // inner list
            for (u64 j = 0; j < sf->func_nodes->length; ++j)
            {
                expr_free_tree(inner_node->data);
                inner_node = inner_node->next;
            }
            list_free(sf->func_nodes, 0);
        }
        x_free(sf);
        outer_node = outer_node->next;
    }

    list_free(scalar_factors, 0);
}

static void free_signature_to_list_of_scalar_factors(struct b64_kv_array* signature_array)
{
    for (u64 i = 0; i < signature_array->length; ++i)
    {
        bitmap_free(signature_array->data[i].key.pointer);
        free_list_of_scalar_factors(signature_array->data[i].value.pointer);
    }
    b64_kv_array_free(signature_array, 0);
}

/*
 * variable_node and non_variable_node can only be leaf nodes or product nodes of leaf nodes
 */
static void* merge_scalars_with_blade(void* scalar_terms, void* blade)
{
    if (scalar_terms == NULL)
    {
        return blade;
    }

    if (blade == NULL)
    {
        return scalar_terms;
    }

    if (expr_is_scalar_node(blade)) // blade = 1.0
    {
        expr_free_tree(blade);
        return scalar_terms;
    }

    if (expr_is_scalar_node(scalar_terms) && (!expr_is_variable_node(scalar_terms)))
    {
        struct leaf_node* scalar_node = scalar_terms;
        if (f64_is_zero(scalar_node->f64_value))
        {
            expr_free_tree(blade);
            return scalar_terms;
        }
        else if (f64_is_close(scalar_node->f64_value, 1.0))
        {
            expr_free_tree(scalar_terms);
            return blade;
        }
    }

    if (expr_is_geometric_product_node(scalar_terms) && expr_is_geometric_product_node(blade))
    {
        struct variadic_node* non_var_node = blade;
        struct expr_node* child = non_var_node->head;
        for (u64 i = 0; i < non_var_node->length; ++i)
        {
            void* _next = child->next;
            expr_variadic_node_append_child(scalar_terms, child);
            child = _next;
        }
        x_free(blade);
        return scalar_terms;
    }

    if (expr_is_geometric_product_node(blade))
    {
        expr_variadic_node_prepend_child(blade, scalar_terms);
        return blade;
    }


    if (expr_is_geometric_product_node(scalar_terms))
    {
        expr_variadic_node_append_child(scalar_terms, blade);
        return scalar_terms;
    }

    struct variadic_node* gp = expr_create_geometric_product_node();
    expr_variadic_node_prepend_child(gp, blade);
    expr_variadic_node_prepend_child(gp, scalar_terms);

    return gp;
}

/*
  The expr should be first expanded and before calling this function, therefore
    the node should be a sum of leaf nodes, and product nodes of leaf nodes or function nodes
*/
static void* expr_reduce_sum_node(void* sum_node)
{
    struct variadic_node* var_node = sum_node;
    struct b64_kv_array* basisblade_to_scalar_factors = b64_kv_array_create(var_node->length);
    struct expr_node* child = var_node->head;

    struct list* unreducable_nodes = list_create();

    for (u64 i = 0; i < var_node->length; ++i)
    {
        void* reduced_child = expr_reduce(child);

        u64 reducable = 1;

        if (expr_is_function_node(reduced_child) && !expr_is_scalar_node(reduced_child))
        {
            reducable = 0;
        }

        if (expr_is_geometric_product_node(reduced_child))
        {
            struct variadic_node* gp = reduced_child;
            struct expr_node* gp_child = gp->head;
            for (u64 j = 0; j < gp->length; ++j)
            {
                if (expr_is_function_node(gp_child) && !expr_is_scalar_node(gp_child))
                {
                    reducable = 0;
                    break;
                }
                gp_child = gp_child->next;
            }
        }

        if (!reducable)
        {
            list_append(unreducable_nodes, reduced_child);
            child = child->next;
            continue;
        }

        void* uninstantiated = NULL;
        void* instantiated = NULL;
        separate_instantiated_uninstantiated(reduced_child, &uninstantiated, &instantiated);
        expr_free_tree(reduced_child);

        void* bb_signature = NULL;
        f64 coeff = 0.0;
        bb_signature = basis_blade_signature(instantiated, &coeff);
        expr_free_tree(instantiated);

        u64 ownership_transferred = 0;
        struct list* list_scalar_factors;
        if (!b64_kv_array_get_ex(basisblade_to_scalar_factors, bb_signature, (union b64*)&list_scalar_factors, bitmap_compare))
        {
            list_scalar_factors = list_create();
            union b64 value = {.pointer = list_scalar_factors};
            b64_kv_array_set_ex(basisblade_to_scalar_factors, bb_signature, value, bitmap_compare, &ownership_transferred);
        }
        if (!ownership_transferred)
        {
            bitmap_free(bb_signature);
        }

        struct scalar_factor* sf = x_calloc(1, sizeof(struct scalar_factor));
        sf->scalar = coeff;

        extract_scalar_factors(uninstantiated, sf);
        expr_free_tree(uninstantiated);

        list_append(list_scalar_factors, sf);

        child = child->next;
    }

    if (basisblade_to_scalar_factors->length == 0)
    {
        free_signature_to_list_of_scalar_factors(basisblade_to_scalar_factors);

        if (unreducable_nodes->length == 0)
        {
            list_free(unreducable_nodes, 0);
            return expr_create_scalar_node_with_f64(0.0);
        }

        if (unreducable_nodes->length == 1)
        {
            struct expr_node* child = unreducable_nodes->head->data;
            list_free(unreducable_nodes, 0);
            return child;
        }

        struct variadic_node* sum_node = expr_create_geometric_sum_node();

        struct list_node* unreducable_node = unreducable_nodes->head;
        for (u64 i = 0; i < unreducable_nodes->length; ++i)
        {
            expr_variadic_node_append_child(sum_node, unreducable_node->data);
            unreducable_node = unreducable_node->next;
        }

        list_free(unreducable_nodes, 0);
        return sum_node;
    }

    struct variadic_node* reduced_sum_node = expr_create_geometric_sum_node();

    if (unreducable_nodes->length > 0)
    {
        struct list_node* unreducable_node = unreducable_nodes->head;
        for (u64 i = 0; i < unreducable_nodes->length; ++i)
        {
            expr_variadic_node_append_child(reduced_sum_node, unreducable_node->data);
            unreducable_node = unreducable_node->next;
        }
    }

    f64 scalar_sum = 0.0;
    struct b64_kv_pair* kv = basisblade_to_scalar_factors->data;
    for (u64 i = 0; i < basisblade_to_scalar_factors->length; ++i)
    {
        void* basis_blade = create_blade_from_signature(kv->key.pointer);
        struct list* list_scalar_factors;
        if (b64_kv_array_get_ex(basisblade_to_scalar_factors, kv->key.pointer, (union b64*)&list_scalar_factors, bitmap_compare))
        {
            void* scalar_terms = create_node_from_list_of_scalar_factors(list_scalar_factors);
            basis_blade = merge_scalars_with_blade(scalar_terms, basis_blade);
        }

        if (expr_is_instantiated_scalar_node(basis_blade))
        {
            scalar_sum += ((struct leaf_node*)basis_blade)->f64_value;
            expr_free_tree(basis_blade);
            kv++;
            continue;
        }

        expr_variadic_node_append_child(reduced_sum_node, basis_blade);

        kv++;
    }

    if (!f64_is_close(scalar_sum, 0.0))
    {
        if (reduced_sum_node->length == 0)
        {
            expr_free_tree(reduced_sum_node);
            free_signature_to_list_of_scalar_factors(basisblade_to_scalar_factors);
            list_free(unreducable_nodes, 0);
            return expr_create_scalar_node_with_f64(scalar_sum);
        }
        expr_variadic_node_prepend_child(reduced_sum_node, expr_create_scalar_node_with_f64(scalar_sum));
    }

    void* flattened = expr_flatten_node_inplace(reduced_sum_node);
    void* cleaned = expr_remove_redundant_nodes(flattened);
    expr_free_tree(flattened);

    free_signature_to_list_of_scalar_factors(basisblade_to_scalar_factors);
    list_free(unreducable_nodes, 0);

    if (!cleaned)
    {
        return expr_create_scalar_node_with_f64(0.0);
    }

    return cleaned;
}
