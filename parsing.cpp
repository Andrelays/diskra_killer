#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/colors.h"
#include "libraries/stack/stack.h"
#include "libraries/onegin/onegin.h"
#include "libraries/utilities/utilities.h"
#include "differentiator.h"
#include "operators.h"

#define syn_assert(condition)                                                                                                           \
 do {                                                                                                                                   \
    if (!(condition))                                                                                                                   \
    {                                                                                                                                   \
        ERROR = 1;                                                                                                                      \
        printf(RED  "File: %s\n"                                                                                                        \
                    "line: %d\n"                                                                                                        \
                    "Function: %s\n"                                                                                                    \
                    "The condition is not met: \"%s\"\n"                                                                                \
                    "Curent symbol is <%c>\n" RESET_COLOR, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition, STRING[STRING_INDEX]);  \
                                                                                                                                        \
        return 0;                                                                                                                       \
    }                                                                                                                                   \
}  while(0)

const char *STRING = NULL;
int STRING_INDEX   = 0;
int ERROR          = 0;

tree_node *get_e();
tree_node *get_t();
tree_node *get_k();
tree_node *get_z();
tree_node *get_x();
tree_node *get_p();
tree_node *get_n();
tree_node *get_v();

// ssize_t input_tree_from_database(FILE *database_file, tree *tree_pointer)
// {
//     MYASSERT(database_file      != NULL, NULL_POINTER_PASSED_TO_FUNC, return DATABASE_FILE_IS_NULL);
//     MYASSERT(tree_pointer       != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_IS_NULL);
//     MYASSERT(tree_pointer->info != NULL, NULL_POINTER_PASSED_TO_FUNC, return POINTER_TO_TREE_INFO_IS_NULL);
//
//     char *database_buffer = saving_buffer(database_file);
//
//     MYASSERT(database_buffer != NULL, NULL_POINTER_PASSED_TO_FUNC, return DATABASE_BUFFER_IS_NULL);
//
//     delete_node(tree_pointer->root);
//     tree_pointer->root = NULL;
//
//     tree_pointer->root = saving_node_from_database(database_buffer, tree_pointer);
//
//     free(database_buffer);
//
//     return VERIFY_TREE(tree_pointer);;
// }

tree_node *get_g(const char *curent_string)
{
    STRING = curent_string;
    STRING_INDEX = 0;
    tree_node *tree_node_pointer = get_e();
    syn_assert(STRING[STRING_INDEX] == '\0');

    return tree_node_pointer;
}

tree_node *get_e()
{
    tree_node *tree_node_pointer = get_t();

    while (STRING[STRING_INDEX] == '<')
    {
        STRING_INDEX++;
        syn_assert(STRING[STRING_INDEX] == '-');
        STRING_INDEX++;
        syn_assert(STRING[STRING_INDEX] == '>');
        STRING_INDEX++;

        tree_node *tree_node_pointer_2 = get_t();

        tree_node_pointer = create_node(OPERATOR, {.operator_index = EQUIVALENCE}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

tree_node *get_t()
{
    tree_node *tree_node_pointer = get_k();

    while (STRING[STRING_INDEX] == '-')
    {
        STRING_INDEX++;
        syn_assert(STRING[STRING_INDEX] == '>');
        STRING_INDEX++;

        tree_node *tree_node_pointer_2 = get_k();

        tree_node_pointer = create_node(OPERATOR, {.operator_index = IMPLICATION}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

tree_node *get_k()
{
    tree_node *tree_node_pointer = get_z();

    while (STRING[STRING_INDEX] == '+' || STRING[STRING_INDEX] == '\\')
    {
        char curent_operator = STRING[STRING_INDEX];
        STRING_INDEX++;

        if (curent_operator == '\\')
        {
            syn_assert(STRING[STRING_INDEX] == '/');
            STRING_INDEX++;
        }

        tree_node *tree_node_pointer_2 = get_z();

        switch(curent_operator)
        {
            case '+':
                tree_node_pointer = create_node(OPERATOR, {.operator_index = XOR}, tree_node_pointer, tree_node_pointer_2);
                break;

            case '\\':
                tree_node_pointer = create_node(OPERATOR, {.operator_index = DISJUNCTION}, tree_node_pointer, tree_node_pointer_2);
                break;

            default:
                syn_assert(0);
        }
    }

    return tree_node_pointer;
}

tree_node *get_z()
{
    tree_node *tree_node_pointer = get_x();

    while (STRING[STRING_INDEX] == '/')
    {
        STRING_INDEX++;
        syn_assert(STRING[STRING_INDEX] == '\\');
        STRING_INDEX++;

        tree_node *tree_node_pointer_2 = get_x();

        tree_node_pointer = create_node(OPERATOR, {.operator_index = CONJUNCTION}, tree_node_pointer, tree_node_pointer_2);
    }

    return tree_node_pointer;
}

tree_node *get_x()
{
    bool is_negation = false;

    if (STRING[STRING_INDEX] == '-') {
        is_negation = true;
        STRING_INDEX++;
    }

    tree_node *tree_node_pointer = get_p();

    if (is_negation) {
        tree_node_pointer = create_node(OPERATOR, {.operator_index = NEGATION}, NULL, tree_node_pointer);
    }

    return tree_node_pointer;
}

tree_node *get_p()
{
    if (STRING[STRING_INDEX] == '(')
    {
        tree_node *tree_node_pointer = NULL;
        STRING_INDEX++;

        tree_node_pointer = get_e();

        syn_assert(STRING[STRING_INDEX] == ')');
        STRING_INDEX++;

        return tree_node_pointer;
    }

    return get_n();
}

tree_node *get_n()
{
    tree_node *tree_node_pointer = NULL;

    int string_index_old = STRING_INDEX;

    if('0' == STRING[STRING_INDEX] || STRING[STRING_INDEX] == '1')
    {
        tree_node_pointer = create_node(NUMBER, {.number = STRING[STRING_INDEX] - '0'}, NULL, NULL);
        STRING_INDEX++;
    }

    syn_assert(string_index_old < STRING_INDEX);

    return tree_node_pointer;
}
