#include <stdio.h>
#include <math.h>
#include "operators.h"
#include "diskran_killer.h"
#include "libraries/utilities/colors.h"
#include "libraries/utilities/utilities.h"

#define UNUSED(x)   \
do {                \
    (void)(x);      \
} while(0)

bool apply_implication(bool left_value, bool right_value)
{
    return (!left_value || right_value);
}

bool apply_equivalence(bool left_value, bool right_value)
{
    return (left_value == right_value);
}

bool apply_negation(bool left_value, bool right_value)
{
    UNUSED(left_value);

    return (!right_value);
}

bool apply_xor(bool left_value, bool right_value)
{
    return ((left_value || right_value) && (!left_value || !right_value));
}

bool apply_disjunction(bool left_value, bool right_value)
{
    return (left_value || right_value);
}

bool apply_conjunction(bool left_value, bool right_value)
{
    return (left_value && right_value);
}
