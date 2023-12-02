#include <stdio.h>
#include <math.h>
#include "operators.h"
#include "differentiator.h"
#include "libraries/utilities/colors.h"
#include "libraries/utilities/utilities.h"

#define UNUSED(x)   \
do {                \
    (void)(x);      \
} while(0)

const double PI = 3.14159265;

double calculation_mul(double left_value, double right_value)
{
    return left_value * right_value;
}

double calculation_add(double left_value, double right_value)
{
    return left_value + right_value;
}

double calculation_sub(double left_value, double right_value)
{
    return left_value - right_value;
}

double calculation_div(double left_value, double right_value)
{
    if (check_equal_with_accuracy(right_value, 0, NEAR_ZERO))
    {
        printf(RED "ERROR! Division by zero.\n" RESET_COLOR);
        return 0;
    }

    return left_value / right_value;
}

double calculation_pow(double left_value, double right_value)
{
    if (check_equal_with_accuracy(left_value, 0, NEAR_ZERO) && check_equal_with_accuracy(right_value, 0, NEAR_ZERO))
    {
        printf(RED "ERROR! Zero to the power of zero.\n" RESET_COLOR);
        return 0;
    }

    return pow(left_value, right_value);
}

double calculation_sin(double left_value, double right_value)
{
    UNUSED(right_value);

    return sin(left_value);
}

double calculation_cos(double left_value, double right_value)
{
    UNUSED(right_value);

    return cos(left_value);
}

double calculation_tan(double left_value, double right_value)
{
    UNUSED(right_value);

    if (check_equal_with_accuracy(left_value, PI / 2, NEAR_ZERO))
    {
        printf(RED "ERROR! Tangent of pi/2.\n" RESET_COLOR);
        return 0;
    }

    return tan(left_value);
}

double calculation_log(double left_value, double right_value)
{
    UNUSED(right_value);

    if (left_value <= 0)
    {
        printf(RED "ERROR! Logarithm of a number less than 0.\n" RESET_COLOR);
        return 0;
    }

    return log(left_value);
}

tree_node *differentiation_add(tree_node *left_node, tree_node *right_node, ssize_t variable_index)
{
    value_node value = {};
    value.operator_index = ADD;

    return create_node(OPERATOR, value, differentiate_node_by_variable(left_node, variable_index), differentiate_node_by_variable(right_node, variable_index));
}
