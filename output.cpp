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

void print_node(FILE *file_output, const tree_node *tree_node_pointer, const variable_parametrs *variable_array, const tree_node *parent_tree_node_pointer)
{
    MYASSERT(file_output                != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(parent_tree_node_pointer   != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(variable_array             != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    if (!tree_node_pointer)
    {
        fprintf(file_output, "_ ");
        return;
    }

    fprintf(file_output,  "( ");

    print_node(file_output, tree_node_pointer->left, variable_array, tree_node_pointer);

    switch(tree_node_pointer->type)
    {
        case VARIABLE:
        {
            fprintf(file_output,  "%s ", variable_array[tree_node_pointer->value.variable_index].name);
            break;
        }

        case OPERATOR:
        {
            fprintf(file_output,  "%s " , OPERATORS[tree_node_pointer->value.operator_index].name);
            break;
        }

        case NUMBER:
        {
            fprintf(file_output, FORMAT_SPECIFIERS_TREE " ", tree_node_pointer->value.number);
            break;
        }

        case PARENTHESIS:
        case NO_TYPE:
        default:
        {
            printf(RED "ERROR! Incorrect data type when output to a file!\n" RESET_COLOR);
            return;
        }
    }

    print_node(file_output, tree_node_pointer->right, variable_array, tree_node_pointer);

    fprintf(file_output, ") ");
}

void print_dnf_form(FILE *file_output, truth_table_info *truth_table, variable_parametrs *variable_array, ssize_t size_variable_array)
{
    MYASSERT(file_output        != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(truth_table        != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(truth_table->array != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(variable_array     != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    bool function_can_represented_in_dnf = false;

    for (ssize_t index_line_truth_tables = 0; index_line_truth_tables < truth_table->size_truth_table; index_line_truth_tables++)
    {
        if (truth_table->array[index_line_truth_tables] == true)
        {
            set_values_for_variables(variable_array, index_line_truth_tables, size_variable_array);

            if (function_can_represented_in_dnf == true) {
                fprintf(file_output, "v ");
            }

            fprintf(file_output, "(");
            function_can_represented_in_dnf = true;


            for (ssize_t index_variable = 0; index_variable < size_variable_array; index_variable++)
            {
                if (variable_array[index_variable].value == true) {
                    fprintf(file_output, "%s", variable_array[index_variable].name);
                }

                else {
                    fprintf(file_output, "¬%s", variable_array[index_variable].name);
                }

                if (index_variable < size_variable_array - 1) {
                    fprintf(file_output, " ^ ");
                }
            }

            fprintf(file_output, ") ");
        }
    }

    if (!function_can_represented_in_dnf) {
        fprintf(file_output, "The function cannot be represented in the dnf\n");
    }

    else {
        fprintf(file_output, "\n");
    }
}
