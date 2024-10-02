/*
 * Copyright 2024 Bin Shao
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "numeric_types.h"
#include "config.h"
#include "mm.h"
#include "symbol.h"
#include "runtime.h"
#include "cli_args.h"
#include "expr.h"
#include "string_util.h"
#include "list.h"
#include "func_repo.h"
#include "print_util.h"

static u64 const CLI_CMD_UNSPECIFIED = 0;
static u64 const CLI_CMD_PRINT_PREFIX = 1;
static u64 const CLI_CMD_PRINT_INFIX = 2;
static u64 const CLI_CMD_PRINT_JSON = 3;
static u64 const CLI_CMD_PRINT_TREE = 4;

static char const* PROMPT = "geomeculus> ";

static struct list named_exprs = {.length = 0, .head = NULL, .tail = NULL};

static struct list completion_list = {.length = 0, .head = NULL, .tail = NULL};

u64 cmd_handler(char* input, char* history_file);

static void cli_expr_evaluate(u64 cmd, char* symbol_name, char* expr_str, struct b64_kv_array* variable_values)
{
    void* expr = NULL;
    u64 cli_expr_input_mode = expr_detect_input_mode(expr_str);

    if (cli_expr_input_mode == EXPR_INPUT_MODE_PREFIX)
    {
        expr = expr_decode_prefix_str(expr_str);
    }
    else if (cli_expr_input_mode == EXPR_INPUT_MODE_INFIX)
    {
        expr = expr_decode_infix_str(expr_str);
    }
    else if (cli_expr_input_mode == EXPR_INPUT_MODE_JSON)
    {
        expr = expr_decode_json_str(expr_str);
    }
    else
    {
        printf("invalid expression encoding mode\n");
        return;
    }

    if (expr == NULL)
    {
        printf("invalid expression\n");
        return;
    }

    void* evaluated_1 = expr_evaluate(expr, NULL);
    void* evaluated_2 = expr_evaluate(evaluated_1, variable_values);

    if (cmd == CLI_CMD_PRINT_INFIX)
    {
        printf("          = ");
        expr_print_infix(expr);
    }
    else if (cmd == CLI_CMD_PRINT_PREFIX)
    {
        printf("          = ");
        expr_print_prefix(expr);
    }
    else if (cmd == CLI_CMD_PRINT_JSON)
    {
        printf("          = ");
        expr_print_json(expr);
    }
    else if (cmd == CLI_CMD_PRINT_TREE)
    {
        expr_print_tree(expr);
    }
    else
    {
        if (!symbol_name)
        {
            printf("          = ");
            expr_print_infix(evaluated_2);
        }
    }

    expr_free_tree(expr);
    expr_free_tree(evaluated_1);

    if (symbol_name)
    {
        struct symbol* symbol = symbol_find_by_name(symbol_name);
        if (symbol)
        {
            if (symbol->type == SYMBOL_TYPE_EXPR)
            {
                expr_free_tree(symbol->pointer);
            }
            else
            {
                printf("Warning: symbol %s exists and is not an expression\n", symbol_name);
                expr_free_tree(evaluated_2);
                return;
            }

            symbol->pointer = evaluated_2;
        }
        else
        {
            symbol = symbol_create(SYMBOL_TYPE_EXPR, symbol_name);
            symbol->pointer = evaluated_2;
        }

        if (!list_contains(&named_exprs, symbol))
        {
            list_append(&named_exprs, symbol);
        }
    }
    else
    {
        expr_free_tree(evaluated_2);
    }
}

static u64 cli_check_cmd(char* input, char* cmd)
{
    if (!input || !cmd)
    {
        return 0;
    }

    u64 len = strlen(cmd);
    if (len < strlen(cmd))
    {
        return 0;
    }

    for (u64 i = 0; i < strlen(cmd); ++i)
    {
        if (input[i] != cmd[i])
        {
            return 0;
        }
    }
    return 1;
}

static void list_callback_print_named_expr(void* symbol)
{
    printf("%s = ", ((struct symbol*)symbol)->name);
    expr_print_infix(((struct symbol*)symbol)->pointer);
}

static void cmd_list_saved_exprs()
{
    printf("%lu saved expressions", named_exprs.length);

    if (named_exprs.length == 0)
    {
        printf("\n");
        return;
    }
    else
    {
        printf(":\n");
    }

    list_iterate(&named_exprs, list_callback_print_named_expr);
}

static u64 valid_filepath(char* path)
{
    if (access(path, F_OK) == 0)
    {
        if (access(path, R_OK) == 0)
        {
            return 1;
        }
        else
        {
            printf("The file '%s' is not readable.\n", path);
        }
    }
    else
    {
        printf("The file '%s' does not exist.\n", path);
    }

    return 0;
}

static void exec_script(char* script_path)
{
    char* trimmed_script_path = string_trim(script_path);
    if (!trimmed_script_path)
    {
        printf("invalid script path\n");
        return;
    }

    if (strlen(trimmed_script_path) == 0)
    {
        printf("invalid script path\n");
        x_free(trimmed_script_path);
        return;
    }

    if (!valid_filepath(trimmed_script_path))
    {
        x_free(trimmed_script_path);
        return;
    }

    FILE* file = fopen(trimmed_script_path, "r");
    x_free(trimmed_script_path);
    if (!file)
    {
        printf("failed to open file: %s\n", trimmed_script_path);
        return;
    }

    printf("executing script: %s\n", trimmed_script_path);

    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1)
    {
        char* trimmed_line = string_trim(line);
        printf("%s%s\n", PROMPT, trimmed_line);
        if(cmd_handler(trimmed_line, NULL))
        {
            x_free(trimmed_line);
            break;
        }
        x_free(trimmed_line);
    }

    free(line);
    fclose(file);
}

static char* cli_completion_generator(char const* text, int state)
{
    static u64 list_index = 0;
    static u64 text_len = 0;

    if (!state)
    {
        list_index = 0;
        text_len = strlen(text);
    }

    while (list_index < completion_list.length)
    {
        char* txt = list_get(&completion_list, list_index);
        list_index += 1;

        if (strncmp(txt, text, text_len) == 0)
        {
            return strdup(txt);
        }
    }

    return NULL;
}

static char** cli_completion(char const* text, int start, int end)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, cli_completion_generator);
}

static void cli_completion_initialize()
{
    rl_readline_name = "geomeculus";

    func_get_names(&completion_list);
    list_append(&completion_list, "help");
    list_append(&completion_list, "display_precision");
    list_append(&completion_list, "dim_signatures");
    list_append(&completion_list, "algebra_signature");
    list_append(&completion_list, "exponential_series_expansion_order");
    list_append(&completion_list, "zeta_series_expansion_order");
    list_append(&completion_list, "print_prefix");
    list_append(&completion_list, "print_infix");
    list_append(&completion_list, "print_json");
    list_append(&completion_list, "print_tree");
    list_append(&completion_list, "list_func");
    list_append(&completion_list, "list_expr");
    list_append(&completion_list, "pwd");
    list_append(&completion_list, "cd");
    list_append(&completion_list, "exec");
    list_append(&completion_list, "exit");
    list_append(&completion_list, "quit");

    rl_attempted_completion_function = cli_completion;
}

static void print_cmd_help(char const* cmd)
{
    if (strcmp(cmd, "help") == 0)
    {
        printf("help [cmd | func_name]\n");
        printf("  <cmd> = name of a command or function\n");
        printf("  prints help for the specified command or function\n");
        printf("  if no argument is provided, prints all available commands\n");
    }
    else if (strcmp(cmd, "<expr>") == 0)
    {
        printf("<expr>\n");
        printf("  a valid expression in infix, prefix, or json notation\n");
        printf("  evaluates the expression and prints the result\n");
    }
    else if (strcmp(cmd, "<symbol> = <expr>") == 0)
    {
        printf("<symbol> = <expr>\n");
        printf("  <symbol> = name of a symbol\n");
        printf("  <expr> = a valid expression in infix, prefix, or json notation\n");
        printf("  evaluates the expression and saves the result to the symbol\n");
    }
    else if (strcmp(cmd, "display_precision") == 0)
    {
        printf("display_precision [value]\n");
        printf("  <value> = u64\n");
        printf("  gets or sets (when value provided) the display precision for floating point numbers\n");
    }
    else if (strcmp(cmd, "dim_signatures") == 0)
    {
        printf("dim_signatures [value pairs]\n");
        printf("  <value pairs> = u64 i32 u64 i32 ...\n");
        printf("  gets or sets (when value provided) the dimension signatures for the algebra context\n");
    }
    else if (strcmp(cmd, "algebra_signature") == 0)
    {
        printf("algebra_signature [values]\n");
        printf("  <values> = [u64], [u64], [u64], [u64]\n");
        printf("  gets or sets (when value(s) provided) the following of the current algebra context:\n");
        printf("    the number of positive\n");
        printf("    the number of negative squares\n");
        printf("    the number of zero squares\n");
        printf("    the starting dim index (1 or 0, default 1)\n");
    }
    else if (strcmp(cmd, "exponential_series_expansion_order") == 0)
    {
        printf("exponential_series_expansion_order [value]\n");
        printf("  <value> = u64\n");
        printf("  gets or sets (when value provided) the number of terms in the series expansion for the exponential function\n");
    }
    else if (strcmp(cmd, "zeta_series_expansion_order") == 0)
    {
        printf("zeta_series_expansion_order [value]\n");
        printf("  <value> = u64\n");
        printf("  gets or sets (when value provided) the number of terms in the series expansion for the zeta function\n");
    }
    else if (strcmp(cmd, "print_prefix") == 0)
    {
        printf("print_prefix <expr>\n");
        printf("  <expr> = expression\n");
        printf("  prints the expression in prefix notation\n");
    }
    else if (strcmp(cmd, "print_infix") == 0)
    {
        printf("print_infix <expr>\n");
        printf("  <expr> = expression\n");
        printf("  prints the expression in infix notation\n");
    }
    else if (strcmp(cmd, "print_json") == 0)
    {
        printf("print_json <expr>\n");
        printf("  <expr> = expression\n");
        printf("  prints the expression in json notation\n");
    }
    else if (strcmp(cmd, "print_tree") == 0)
    {
        printf("print_tree <expr>\n");
        printf("  <expr> = expression\n");
        printf("  prints the expression in tree notation\n");
    }
    else if (strcmp(cmd, "list_func") == 0)
    {
        printf("list_func\n");
        printf("  lists all functions in the function repository\n");
    }
    else if (strcmp(cmd, "list_expr") == 0)
    {
        printf("list_expr\n");
        printf("  lists all saved expressions\n");
    }
    else if (strcmp(cmd, "pwd") == 0)
    {
        printf("pwd\n");
        printf("  prints the current working directory\n");
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        printf("cd <path>\n");
        printf("  <path> = path to a directory\n");
        printf("  changes the current working directory\n");
    }
    else if (strcmp(cmd, "exec") == 0)
    {
        printf("exec <script_path>\n");
        printf("  <script_path> = path to a script file\n");
        printf("  executes the script file\n");
    }
    else if (strcmp(cmd, "exit") == 0)
    {
        printf("exit\n");
        printf("  exits the program (equivalent to quit)\n");
    }
    else if (strcmp(cmd, "quit") == 0)
    {
        printf("quit\n");
        printf("  quits the program (equivalent to exit)\n");
    }
    else
    {
        printf("unknown command: %s\n", cmd);
    }
}

void print_feedback_channel()
{
    print_block_line(NULL);
    printf("          Feedback and Suggestions: Bin Shao <binshao@microsoft.com>\n");
    print_block_line(NULL);
}

u64 cmd_handler(char* trimmed_input, char* history_file)
{
    if (!trimmed_input)
    {
        return 0;
    }

    // comment line
    if (cli_check_cmd(trimmed_input, "#") || cli_check_cmd(trimmed_input, "//"))
    {
        return 0;
    }

    if (cli_check_cmd(trimmed_input, "exit") || cli_check_cmd(trimmed_input, "quit"))
    {
        return 1;
    }

    add_history(trimmed_input);
    if (history_file)
    {
        write_history(history_file);
    }

    if (cli_check_cmd(trimmed_input, "help"))
    {
        u64 cmd_len = strlen("help");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (arg_str)
        {
            char* value = string_trim(arg_str);
            x_free(arg_str);

            if (value)
            {
                if(!func_get_help(value))
                {
                    print_cmd_help(value);
                }
                x_free(value);
            }

            return 0;
        }

        printf("geomeculus commands:\n");
        printf("  <expr>\n");
        printf("  <symbol> = <expr>\n");
        printf("  display_precision [value]\n");
        printf("  dim_signatures [value pairs]\n");
        printf("  algebra_signature [value]\n");
        printf("  exponential_series_expansion_order [value]\n");
        printf("  zeta_series_expansion_order [value]\n");
        printf("  print_prefix <expr>\n");
        printf("  print_infix <expr>\n");
        printf("  print_json <expr>\n");
        printf("  print_tree <expr>\n");
        printf("  list_func\n");
        printf("  list_expr\n");
        printf("  pwd\n");
        printf("  cd <path>\n");
        printf("  exec <script_path>\n");
        printf("  exit\n");
        printf("  quit\n");

        return 0;
    }

    if (cli_check_cmd(trimmed_input, "pwd"))
    {
        printf("%s\n", working_directory());
        return 0;
    }

    if (cli_check_cmd(trimmed_input, "cd"))
    {
        u64 cmd_len = strlen("cd");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (arg_str)
        {
            char* value = string_trim(arg_str);
            x_free(arg_str);

            if (value)
            {
                if (chdir(value) != 0)
                {
                    printf("failed to change directory to: %s\n", value);
                }
                x_free(value);
            }

            return 0;
        }

        return 0;
    }

    if (cli_check_cmd(trimmed_input, "exec"))
    {
        u64 cmd_len = strlen("exec");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (arg_str)
        {
            exec_script(arg_str);
            x_free(arg_str);
        }

        return 0;
    }

    if (cli_check_cmd(trimmed_input, "list_func"))
    {
        func_print_repo();
        return 0;
    }

    if (cli_check_cmd(trimmed_input, "list_expr"))
    {
        cmd_list_saved_exprs();
        return 0;
    }

    u64 cli_cmd = CLI_CMD_UNSPECIFIED;

    if (cli_check_cmd(trimmed_input, "display_precision"))
    {
        u64 cmd_len = strlen("display_precision");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (!arg_str)
        {
            printf("display_precision = %lu\n", f64_display_precision);
            return 0;
        }

        char* value = string_trim(arg_str);
        x_free(arg_str);
        if (value)
        {
            f64_display_precision = strtoull(value, NULL, 10);
            x_free(value);
        }
        return 0;
    }

    if (cli_check_cmd(trimmed_input, "dim_signatures"))
    {
        u64 cmd_len = strlen("dim_signatures");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (!arg_str)
        {
            algebra_ctx_print(algebra_ctx);
            return 0;
        }

        char* value = string_trim(arg_str);
        x_free(arg_str);

        if (value)
        {
            struct list* sigs = string_split(value, " ", 1);
            if (sigs->length % 2 != 0)
            {
                printf("invalid signature list\n");
                list_free(sigs, 1);
                x_free(value);
                return 0;
            }

            if (algebra_ctx)
            {
                algebra_ctx_free(algebra_ctx);
            }

            algebra_ctx = x_malloc(sizeof(struct algebra_context));
            algebra_ctx->dim_count = sigs->length >> 1;
            algebra_ctx->dim_indexes = x_malloc(algebra_ctx->dim_count * sizeof(u64));
            algebra_ctx->dim_signatures = x_malloc(algebra_ctx->dim_count * sizeof(i32));

            for (u64 i = 0; i < sigs->length; i += 2)
            {
                char* idx_str = list_get(sigs, i);
                char* sig_str = list_get(sigs, i + 1);

                u64 idx = strtoull(idx_str, NULL, 10);
                i32 sig = strtol(sig_str, NULL, 10);

                u64 p = i >> 1;

                if (p < algebra_ctx->dim_count)
                {
                    algebra_ctx->dim_indexes[p] = idx;
                    algebra_ctx->dim_signatures[p] = sig;
                }
                else
                {
                    printf("invalid dimension index: %lu\n", idx);
                }
            }

            list_free(sigs, 1);
            x_free(value);
        }

        return 0;
    }

    if (cli_check_cmd(trimmed_input, "algebra_signature"))
    {
        u64 cmd_len = strlen("algebra_signature");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (!arg_str)
        {
            algebra_ctx_print(algebra_ctx);
            return 0;
        }

        char* value = string_trim(arg_str);
        x_free(arg_str);

        if (value)
        {
            struct list* sigs = string_split(value, ",", 1);
            if (sigs->length == 0)
            {
                list_free(sigs, 1);
                x_free(value);
                return 0;
            }

            u64 num_positive_square = 0;
            u64 num_negative_square = 0;
            u64 num_zero_square = 0;
            u64 starting_dim_index = 1;

            if (sigs->length >= 1)
            {
                num_positive_square = strtoull(list_get(sigs, 0), NULL, 10);
            }

            if (sigs->length >= 2)
            {
                num_negative_square = strtoull(list_get(sigs, 1), NULL, 10);
            }

            if (sigs->length >= 3)
            {
                num_zero_square = strtoull(list_get(sigs, 2), NULL, 10);
            }

            if (sigs->length >= 4)
            {
                starting_dim_index = strtoull(list_get(sigs, 3), NULL, 10);
            }

            algebra_ctx_create_from_signature(num_positive_square, num_negative_square, num_zero_square, starting_dim_index);

            list_free(sigs, 1);
            x_free(value);
        }

        return 0;
    }

    if (cli_check_cmd(trimmed_input, "exponential_series_expansion_order"))
    {
        u64 cmd_len = strlen("exponential_series_expansion_order");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (!arg_str)
        {
            printf("exponential_series_expansion_order = %lu\n", exponential_series_expansion_order);
            return 0;
        }

        char* value = string_trim(arg_str);
        x_free(arg_str);
        if (value)
        {
            exponential_series_expansion_order = strtoull(value, NULL, 10);
            x_free(value);
        }
        return 0;
    }

    if (cli_check_cmd(trimmed_input, "zeta_series_expansion_order"))
    {
        u64 cmd_len = strlen("zeta_series_expansion_order");
        char* arg_str = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));

        if (!arg_str)
        {
            printf("zeta_series_expansion_order = %lu\n", zeta_series_expansion_order);
            return 0;
        }

        char* value = string_trim(arg_str);
        x_free(arg_str);
        if (value)
        {
            zeta_series_expansion_order = strtoull(value, NULL, 10);
            x_free(value);
        }
        return 0;
    }

    char* _expr = NULL;
    if (cli_check_cmd(trimmed_input, "print_prefix"))
    {
        u64 cmd_len = strlen("print_prefix");
        _expr = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));
        cli_cmd = CLI_CMD_PRINT_PREFIX;
    }
    else if (cli_check_cmd(trimmed_input, "print_infix"))
    {
        u64 cmd_len = strlen("print_infix");
        _expr = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));
        cli_cmd = CLI_CMD_PRINT_INFIX;
    }
    else if (cli_check_cmd(trimmed_input, "print_json"))
    {
        u64 cmd_len = strlen("print_json");
        _expr = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));
        cli_cmd = CLI_CMD_PRINT_JSON;
    }
    else if (cli_check_cmd(trimmed_input, "print_tree"))
    {
        u64 cmd_len = strlen("print_tree");
        _expr = string_substring(trimmed_input, cmd_len, strlen(trimmed_input));
        cli_cmd = CLI_CMD_PRINT_TREE;
    }

    struct list* cols = NULL;

    if (_expr)
    {
        cols = string_split(_expr, ";", 1);
        x_free(_expr);
    }
    else
    {
        cols = string_split(trimmed_input, ";", 1);
    }

    if (!cols)
    {
        return 0;
    }

    if (cols->length == 0)
    {
        list_free(cols, 1);
        return 0;
    }

    char* s = list_get(cols, 0);

    if (!s)
    {
        list_free(cols, 1);
        return 0;
    }

    struct list* symbol_expr = string_split(s, "=", 1);

    if (!symbol_expr)
    {
        list_free(cols, 1);
        return 0;
    }

    char* sym_name = NULL;
    char* expr_str = NULL;

    if (symbol_expr->length == 1)
    {
        sym_name = NULL;
        expr_str = list_get(symbol_expr, 0);
    }

    if (symbol_expr->length == 2)
    {
        sym_name = list_get(symbol_expr, 0);
        expr_str = list_get(symbol_expr, 1);
    }

    if (!expr_str)
    {
        list_free(cols, 1);
        list_free(symbol_expr, 1);
        return 0;
    }

    if (cols->length > 1)
    {
        struct b64_kv_array* variable_values = b64_kv_array_create(cols->length - 1);

        union b64 key;
        union b64 value;

        for (u64 i = 1; i < cols->length; ++i)
        {
            char* assignment = list_get(cols, i);
            struct list* kv = string_split(assignment, "=", 1);
            if (kv->length != 2)
            {
                printf("invalid assignment: %s\n", assignment);
                return 0;
            }

            char* k = list_get(kv, 0);
            char* v = list_get(kv, 1);

            key.u64_value = strtoull(k + 1, NULL, 10);
            value.f64_value = strtod(v, NULL);
            b64_kv_array_set(variable_values, key, value);

            list_free(kv, 1);
        }

        cli_expr_evaluate(cli_cmd, sym_name, expr_str, variable_values);
        b64_kv_array_free(variable_values, 0);
    }
    else
    {
        cli_expr_evaluate(cli_cmd, sym_name, expr_str, NULL);
    }

    list_free(cols, 1);
    list_free(symbol_expr, 1);

    return 0;
}

static u64 data_on_stdin()
{
    u64 result = 0;
    if (isatty(fileno(stdin)))
    {
        result = 0;
    }
    else
    {
        result = 1;
    }
    return result;
}

static void command_line_interface(int argc, char *argv[])
{
    // handling the input given via pipe
    if (data_on_stdin())
    {
        char* line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline(&line, &len, stdin)) != -1)
        {
            char* trimmed_line = string_trim(line);
            printf("%s%s\n", PROMPT, trimmed_line);
            if(cmd_handler(trimmed_line, NULL))
            {
                x_free(trimmed_line);
                break;
            }
            x_free(trimmed_line);
        }
        free(line);

        return;
    }

    using_history();
    char* history_file = history_file_path();
    read_history(history_file);

    while (1)
    {
        char* input = readline(PROMPT);
        char* trimmed_input = string_trim(input);
        free(input);

        if (cmd_handler(trimmed_input, history_file))
        {
            x_free(trimmed_input);
            break;
        }
        x_free(trimmed_input);
    }

    clear_history();
}

void list_callback_free_named_expr(void* symbol)
{
    expr_free_tree(((struct symbol*)symbol)->pointer);
}

void free_named_expr()
{
    list_iterate(&named_exprs, list_callback_free_named_expr);
    list_clear(&named_exprs, 0);
}

int main(int argc, char *argv[])
{
    runtime_initialize();

    func_register("expand", func_expand, NULL, FUNC_FLAG_NONE,
    "(<expr>): expands a multivector");

    func_register("reduce", func_reduce, NULL, FUNC_FLAG_NONE,
    "(<expr>): reduces a multivector");

    func_register("expand_reduce", func_expand_reduce, NULL, FUNC_FLAG_NONE,
    "(<expr>): expands then reduces a multivector");

    func_register("grade", func_grade, NULL, FUNC_FLAG_NONE,
    "(<expr>, u64 k): grade of a multivector");

    func_register("reverse", func_reverse, NULL, FUNC_FLAG_NONE,
    "(<expr>): reverse of a multivector");

    func_register("inverse_blade", func_inverse_blade, NULL, FUNC_FLAG_NONE,
    "(<expr>): inverse of a nonzero r-blade");

    func_register("magnitude", func_magnitude, NULL, FUNC_FLAG_NONE,
    "(<expr>): magnitude of a multivector");

    func_register("magnitude_squared", func_magnitude_squared, NULL, FUNC_FLAG_NONE,
    "(<expr>): magnitude squared of a multivector");

    func_register("inner_product", func_inner_product, NULL, FUNC_FLAG_NONE,
    "(<expr> exprs ...): inner product of a list of multivectors");

    func_register("outer_product", func_outer_product, NULL, FUNC_FLAG_NONE,
    "(<expr> exprs ...): outer product of a list of multivectors");

    func_register("scalar_product", func_scalar_product, NULL, FUNC_FLAG_NONE,
    "(<expr> A, <expr> B): scalar product of two multivectors A and B");

    func_register("commutator_product", func_commutator_product, NULL, FUNC_FLAG_NONE,
    "(<expr> A, <expr> B): commutator product of two multivectors A and B");

    func_register("anticommutator_product", func_anticommutator_product, NULL, FUNC_FLAG_NONE,
    "(<expr> A, <expr> B): anticommutator product of two multivectors A and B");

    func_register("power", func_power, NULL, FUNC_FLAG_NONE,
    "(<expr> expr, u64 n): power of a multivector expr to the nth power");

    func_register("project", func_project, NULL, FUNC_FLAG_NONE,
    "(<expr> b, <expr> A): projection of vector b onto A");

    func_register("reject", func_reject, NULL, FUNC_FLAG_NONE,
    "(<expr> b, <expr> A): rejection of vector b onto A");

    func_register("partial_derivative", func_partial_derivative, NULL, FUNC_FLAG_NONE,
    "(<expr>, u64 var_idx): the partial derivative of expr with respect to the variable specified by var_idx");

    func_register("nth_partial_derivative", func_nth_partial_derivative, NULL, FUNC_FLAG_NONE,
    "(<expr>, u64 var_idx, u64 n): the nth partial derivative of expr with respect to the variable specified by var_idx");

    func_register("gradient", func_gradient, NULL, FUNC_FLAG_NONE,
    "(<expr>, u64 n): the gradient of expr defined on n-dimensional space");

    func_register("vector_derivative", func_vector_derivative, NULL, FUNC_FLAG_NONE,
    "(<expr> F, <expr> M, u64 m): vector derivative of a field F defined on m-dimensional manifold M");

    func_register("exp_3d", func_exp_3d, NULL, FUNC_FLAG_NONE,
    "(<expr>): Exponentiates a multivector in 3 dimensional space");

    func_register("exponential", func_exponential, NULL, FUNC_FLAG_NONE,
    "(<expr>, [u64 n]): Exponentiates a multivector, the number of terms in the series expansion can be specified by n");

    func_register("rotate", func_rotate_vector_in_plane, NULL, FUNC_FLAG_NONE,
    "(<expr> v, <expr> B, f64 theta): Rotates vector v by an angle theta in a plane specified by bivector B");

    cli_completion_initialize();

    if (argc == 1 || cli_check_flag(argc, argv, "--cli"))
    {
        command_line_interface(argc, argv);
    }
    else if (cli_check_flag(argc, argv, "--import"))
    {
        char* script_path = cli_get_arg_value_by_key(argc, argv, "--import");
        exec_script(script_path);
        command_line_interface(argc, argv);
    }
    else if (cli_check_flag(argc, argv, "--help"))
    {
        printf("geomeculus [<script_path>] [--cli] [--import <script_path>] [--help]\n");
        printf("  <script_path>: execute the script file and exit\n");
        printf("  --cli: run the command line interface\n");
        printf("  --import <script_path>: import the script file and run the command line interface\n");
        printf("  --help: print this help message\n");

        print_feedback_channel();
    }
    else if (argc == 2)
    {
        exec_script(argv[1]);
    }

    list_clear(&completion_list, 0);
    free_named_expr();
    algebra_ctx_free(algebra_ctx);
    runtime_finalize();
    return 0;
}
