#include <math.h>
#include "libraries/utilities/myassert.h"
#include "libraries/utilities/colors.h"
#include "libraries/stack/stack.h"
#include "libraries/onegin/onegin.h"
#include "libraries/utilities/utilities.h"
#include "diskran_killer.h"
#include "dsl.h"
#include "operators.h"

truth_table_info *truth_table_constructor(tree *tree_pointer)
{
    MYASSERT(tree_pointer != NULL, NULL_POINTER_PASSED_TO_FUNC, return NULL);

    ssize_t size_truth_table = (ssize_t) pow(2, tree_pointer->variable_array_position);

    truth_table_info *truth_table = (truth_table_info *) calloc(1, sizeof(truth_table_info));
    MYASSERT(truth_table != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    truth_table->array = (bool *) calloc((size_t) size_truth_table, sizeof(bool));
    MYASSERT(truth_table->array != NULL, FAILED_TO_ALLOCATE_DYNAM_MEMOR, return NULL);

    for (ssize_t index_line_truth_tables = 0; index_line_truth_tables < size_truth_table; index_line_truth_tables++)
    {
        set_values_for_variables(tree_pointer->variable_array, index_line_truth_tables, tree_pointer->variable_array_position);

        truth_table->array[index_line_truth_tables] = calcucalate_node(tree_pointer->root, tree_pointer->variable_array);
    }

    truth_table->size_truth_table = size_truth_table;

    return truth_table;
}

void truth_table_destructor(truth_table_info *truth_table)
{
    MYASSERT(truth_table        != NULL, NULL_POINTER_PASSED_TO_FUNC, return);
    MYASSERT(truth_table->array != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    free(truth_table->array);
    truth_table->array = NULL;
    truth_table->size_truth_table = -1;

    free(truth_table);
}

void set_values_for_variables(variable_parametrs *variable_array, ssize_t index_line_truth_tables, ssize_t size_variable_array)
{
    MYASSERT(variable_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return);

    for (ssize_t index_variable = size_variable_array - 1; index_variable > -1; index_variable--)
    {
        variable_array[index_variable].value = (index_line_truth_tables & 1);
        index_line_truth_tables >>= 1;
    }
}

bool calcucalate_node(tree_node *tree_node_pointer, const variable_parametrs *variable_array)
{
    MYASSERT(variable_array != NULL, NULL_POINTER_PASSED_TO_FUNC, return false);

    if (!tree_node_pointer) {
        return false;
    }

    if (tree_node_pointer->type == NUMBER) {
        return tree_node_pointer->value.number;
    }

    if (tree_node_pointer->type == VARIABLE) {
        return CURRENT_VARIABLE.value;
    }

    bool left_value  = calcucalate_node(tree_node_pointer->left,  variable_array);
    bool right_value = calcucalate_node(tree_node_pointer->right, variable_array);

    return CURRENT_CALCULATE_FUNC(left_value, right_value);
}
