#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/colors.h"
#include "libraries/stack/stack.h"
#include "libraries/onegin/onegin.h"
#include "libraries/utilities/utilities.h"
#include "diskran_killer.h"
#include "operators.h"
#include "dsl.h"

const ssize_t NO_OPERATOR = -1;
const ssize_t NO_VARIABLE = -1;

#define syn_assert(condition)                                                                                                           \
 do {                                                                                                                                   \
    if (!(condition))                                                                                                                   \
    {                                                                                                                                   \
        printf(RED  "File: %s\n"                                                                                                        \
                    "line: %d\n"                                                                                                        \
                    "Function: %s\n"                                                                                                    \
                    "The condition is not met: \"%s\"\n"RESET_COLOR, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition);              \
                                                                                                                                        \
        return 0;                                                                                                                       \
    }                                                                                                                                   \
}  while(0)

static tree_node *get_grammar    (parsing_info *, tree *);
static tree_node *get_equivalence(parsing_info *, tree *);
static tree_node *get_implication(parsing_info *, tree *);
static tree_node *get_logical_or (parsing_info *, tree *);
static tree_node *get_logical_and(parsing_info *, tree *);
static tree_node *get_negation   (parsing_info *, tree *);
static tree_node *get_parenthesis(parsing_info *, tree *);
static tree_node *get_number     (parsing_info *, tree *);
static tree_node *get_variable   (parsing_info *, tree *);

static char         *saving_buffer           (FILE *database_file, size_t *buffer_size);
static parsing_info *parsing_info_constructor(tree *tree_pointer, const char *database_buffer, size_t buffer_size);
static ssize_t       find_operator           (const char *current_token);
static ssize_t       find_variable           (const char *current_token, variable_parametrs *variable_array, ssize_t variable_array_size);
static bool          add_variable            (const char *current_token, variable_parametrs *variable_array, ssize_t *variable_array_position_pointer);
static void          check_size_token_array  (parsing_info *parsing);
static void          parsing_info_destructor (parsing_info *parsing);

ssize_t input_tree_from_database(FILE *database_file, tree *tree_pointer)
{
    MYASSERT(database_file      != NULL, NULL_POINTER_PASSED_TO_FUNC, return DATABASE_FILE_IS_NULL);
    MYASSERT(tree_pointer       != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
    MYASSERT(tree_pointer->info != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);

    size_t buffer_size = 0;

    char *database_buffer = saving_buffer(database_file, &buffer_size);
    MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return DATABASE_BUFFER_IS_NULL);

    delete_node(tree_pointer->root);
    tree_pointer->root = NULL;
    tree_pointer->size = 0;
    tree_pointer->variable_array_position = 0;

    parsing_info *parsing = parsing_info_constructor(tree_pointer, database_buffer, buffer_size);

    tree_pointer->root = get_grammar(parsing, tree_pointer);

    parsing_info_destructor(parsing);
    free(database_buffer);

    return VERIFY_TREE(tree_pointer);;
}

static parsing_info *parsing_info_constructor(tree *tree_pointer, const char *database_buffer, size_t buffer_size)
{
    MYASSERT(database_buffer    != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer       != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer->info != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    const ssize_t INITIAL_SIZE_TOKENS_ARRAY = 100;

    parsing_info *parsing = (parsing_info *) calloc(1, sizeof(parsing_info));;

    parsing->size_token_array = INITIAL_SIZE_TOKENS_ARRAY;
    parsing->token_array_position = 0;

    parsing->token_array = (token_info *) calloc(INITIAL_SIZE_TOKENS_ARRAY, sizeof(token_info));

    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    for (size_t buffer_position = 0; buffer_position < buffer_size; buffer_position++)
    {
        check_size_token_array(parsing);

        ssize_t operator_index = NO_OPERATOR;
        ssize_t variable_index = NO_VARIABLE;
        if (database_buffer[buffer_position] == '(' || database_buffer[buffer_position] == ')')
        {
            CURRENT_TOKEN.type = PARENTHESIS;
            CURRENT_TOKEN.value.is_closing_parenthesis = database_buffer[buffer_position] - '(';
            ++parsing->token_array_position;

            continue;
        }

        if (database_buffer[buffer_position] == '1' || database_buffer[buffer_position] == '0')
        {
            CURRENT_TOKEN.type = NUMBER;
            CURRENT_TOKEN.value.number = database_buffer[buffer_position] - '0';
            ++parsing->token_array_position;

            continue;
        }

        if ((operator_index = find_operator(database_buffer + buffer_position)) != NO_OPERATOR)
        {
            CURRENT_TOKEN.type = OPERATOR;
            CURRENT_TOKEN.value.operator_index = operator_index;
            ++parsing->token_array_position;

            continue;
        }

        if (isalpha(database_buffer[buffer_position]))
        {
            char variable_name[MAX_SIZE_NAME_VARIABLE] = "";

            while(isalnum(database_buffer[buffer_position]))
            {
                size_t index_variable_name = 0;

                if (index_variable_name < MAX_SIZE_NAME_VARIABLE) {
                    variable_name[index_variable_name] = database_buffer[buffer_position];
                    ++buffer_position;
                }

                else
                {
                    while(isalnum(database_buffer[buffer_position]))
                    {
                        ++buffer_position;
                        continue;
                    }
                }
            }

            --buffer_position;

            if ((variable_index = find_variable(variable_name, tree_pointer->variable_array, tree_pointer->variable_array_position)) == NO_VARIABLE)
            {
                if (!add_variable(variable_name, tree_pointer->variable_array, &tree_pointer->variable_array_position))
                {
                    printf(RED "Error! Too many variables\n" RESET_COLOR);
                    continue;
                }

                variable_index = tree_pointer->variable_array_position - 1;
            }

            CURRENT_TOKEN.type = VARIABLE;
            CURRENT_TOKEN.value.variable_index = variable_index;
            ++parsing->token_array_position;

            continue;
        }
    }

    parsing->size_token_array = parsing->token_array_position;
    parsing->token_array_position = 0;

    return parsing;
}

static void parsing_info_destructor(parsing_info *parsing)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    free(parsing->token_array);

    parsing->token_array          = NULL;
    parsing->size_token_array     = -1;
    parsing->token_array_position = -1;

    free(parsing);

    parsing = NULL;
}

static ssize_t find_operator(const char *current_token)
{
    MYASSERT(current_token != NULL, NULL_POINTER_PASSED_TO_FUNC, return NO_OPERATOR);

    ssize_t size_operators_array = sizeof(OPERATORS) / sizeof(*OPERATORS);

    for (ssize_t operator_index = 0; operator_index < size_operators_array; operator_index++)
    {
        if (strncmp(OPERATORS[operator_index].name, current_token, 1) == 0) {
            return operator_index;
        }
    }

    return NO_OPERATOR;
}

static ssize_t find_variable(const char *current_token, variable_parametrs *variable_array, ssize_t variable_array_size)
{
    MYASSERT(current_token  != NULL, NULL_POINTER_PASSED_TO_FUNC, return NO_VARIABLE);
    MYASSERT(variable_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NO_VARIABLE);

    for (ssize_t variable_index = 0; variable_index < variable_array_size; variable_index++)
    {
        if (strcmp((variable_array[variable_index]).name, current_token) == 0) {
            return variable_index;
        }
    }

    return NO_VARIABLE;
}

static bool add_variable(const char *current_token, variable_parametrs *variable_array, ssize_t *variable_array_position_pointer)
{
    MYASSERT(current_token  != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);
    MYASSERT(variable_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    ssize_t variable_array_position = *variable_array_position_pointer;

    if ((size_t) variable_array_position >= SIZE_ARRAY_OF_VARIABLE)
    {
        printf(RED "ERROR! Too many variable. SIZE_ARRAY_OF_VARIABLE = %lu.\n" RESET_COLOR, SIZE_ARRAY_OF_VARIABLE);
        return false;
    }

    strncpy(variable_array[variable_array_position].name, current_token, MAX_SIZE_NAME_VARIABLE);

    ++(*variable_array_position_pointer);

    return true;
}

static void check_size_token_array(parsing_info *parsing)
{
    MYASSERT(parsing != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    if (parsing->token_array_position >= parsing->size_token_array)
    {
        parsing->size_token_array *= 2;
        parsing->token_array = (token_info *) realloc(parsing->token_array, sizeof(token_info) * (size_t) parsing->size_token_array);
    }
}

static char *saving_buffer(FILE *database_file, size_t *buffer_size)
{
    MYASSERT(database_file != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    *buffer_size = determine_size(database_file);

    char *database_buffer = (char *)calloc(*buffer_size + 1, sizeof(char));

    MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    *buffer_size = fread(database_buffer, sizeof(char), *buffer_size, database_file);
    database_buffer[*buffer_size] = '\0';

    return database_buffer;
}


static tree_node *get_grammar(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);


    tree_node *tree_node_pointer = get_equivalence(parsing, tree_pointer);

    syn_assert(parsing->token_array_position == parsing->size_token_array);

    return tree_node_pointer;
}

static tree_node *get_equivalence(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = get_implication(parsing, tree_pointer);

    while (CURRENT_TOKEN.type == OPERATOR && CURRENT_TOKEN.value.operator_index == EQUIVALENCE)
    {
        ++parsing->token_array_position;
        ++tree_pointer->size;
        tree_node *tree_node_pointer_2 = get_implication(parsing, tree_pointer);

        tree_node_pointer = create_node(OPERATOR, {.operator_index = EQUIVALENCE}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

static tree_node *get_implication(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = get_logical_or(parsing, tree_pointer);

    while (CURRENT_TOKEN.type == OPERATOR && CURRENT_TOKEN.value.operator_index == IMPLICATION)
    {
        ++parsing->token_array_position;
        ++tree_pointer->size;
        tree_node *tree_node_pointer_2 = get_logical_or(parsing, tree_pointer);

        tree_node_pointer = create_node(OPERATOR, {.operator_index = IMPLICATION}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

static tree_node *get_logical_or(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = get_logical_and(parsing, tree_pointer);

    while (CURRENT_TOKEN.type == OPERATOR && (CURRENT_TOKEN.value.operator_index == XOR || CURRENT_TOKEN.value.operator_index == DISJUNCTION))
    {
        ssize_t operator_index = CURRENT_TOKEN.value.operator_index;
        ++parsing->token_array_position;
        ++tree_pointer->size;

        tree_node *tree_node_pointer_2 = get_logical_and(parsing, tree_pointer);

        switch(operator_index)
        {
            case XOR:
                tree_node_pointer = create_node(OPERATOR, {.operator_index = XOR}, tree_node_pointer, tree_node_pointer_2);
                break;

            case DISJUNCTION:
                tree_node_pointer = create_node(OPERATOR, {.operator_index = DISJUNCTION}, tree_node_pointer, tree_node_pointer_2);
                break;

            default:
                syn_assert(0);
        }
    }

    return tree_node_pointer;
}

static tree_node *get_logical_and(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = get_negation(parsing, tree_pointer);

    while (CURRENT_TOKEN.type == OPERATOR && CURRENT_TOKEN.value.operator_index == CONJUNCTION)
    {
        ++parsing->token_array_position;
        ++tree_pointer->size;

        tree_node *tree_node_pointer_2 = get_negation(parsing, tree_pointer);

        tree_node_pointer = create_node(OPERATOR, {.operator_index = CONJUNCTION}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

static tree_node *get_negation(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    bool is_negation = false;

    if (CURRENT_TOKEN.type == OPERATOR && CURRENT_TOKEN.value.operator_index == NEGATION)
    {
        is_negation = true;
        ++parsing->token_array_position;
        ++tree_pointer->size;
    }

    tree_node *tree_node_pointer = get_parenthesis(parsing, tree_pointer);

    if (is_negation) {
        tree_node_pointer = create_node(OPERATOR, {.operator_index = NEGATION}, NULL, tree_node_pointer);
    }

    return tree_node_pointer;
}

static tree_node *get_parenthesis(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = NULL;

    if (CURRENT_TOKEN.type == PARENTHESIS && CURRENT_TOKEN.value.is_closing_parenthesis == false)
    {
        ++parsing->token_array_position;

        tree_node_pointer = get_equivalence(parsing, tree_pointer);

        syn_assert(CURRENT_TOKEN.type == PARENTHESIS && CURRENT_TOKEN.value.is_closing_parenthesis == true);
        ++parsing->token_array_position;

        return tree_node_pointer;
    }

    if (!(tree_node_pointer = get_number(parsing, tree_pointer))) {
        tree_node_pointer = get_variable(parsing, tree_pointer);
    }

    return tree_node_pointer;
}

static tree_node *get_number(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = NULL;

    if(CURRENT_TOKEN.type == NUMBER)
    {
        tree_node_pointer = create_node(NUMBER, CURRENT_TOKEN.value, NULL, NULL);
        ++parsing->token_array_position;
        ++tree_pointer->size;
    }

    return tree_node_pointer;
}

tree_node *get_variable(parsing_info *parsing, tree *tree_pointer)
{
    MYASSERT(parsing              != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(parsing->token_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);
    MYASSERT(tree_pointer         != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    tree_node *tree_node_pointer = NULL;

    ssize_t string_index_old = parsing->token_array_position;

    if(CURRENT_TOKEN.type == VARIABLE)
    {
        tree_node_pointer = create_node(VARIABLE, CURRENT_TOKEN.value, NULL, NULL);
        ++parsing->token_array_position;
        ++tree_pointer->size;
    }

    syn_assert(string_index_old < parsing->token_array_position);

    return tree_node_pointer;
}
