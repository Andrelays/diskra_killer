#ifndef DSL_H_INCLUDED
#define DSL_H_INCLUDED

#define  CURRENT_VARIABLE           variable_array[tree_node_pointer->value.variable_index]
#define  CURRENT_CALCULATE_FUNC     OPERATORS[tree_node_pointer->value.operator_index].current_logical_func
#define  CURRENT_DIFFERENTIATE_FUNC OPERATORS[tree_node_pointer->value.operator_index].differentiate_func
#define  VALUE_OF_NODE              tree_node_pointer->value
#define  CURRENT_TOKEN              parsing->token_array[parsing->token_array_position]

#endif
