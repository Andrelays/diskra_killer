#ifndef OPERATORS_H_INCLUDED
#define OPERATORS_H_INCLUDED

#include "diskran_killer.h"

typedef bool (*logical_function) (bool, bool);

struct operator_parametrs {
    const char              *name;
    bool                     is_binary;
    logical_function         current_logical_func;
    size_t                   priority;
};

bool apply_implication(bool left_value, bool right_value);
bool apply_equivalence(bool left_value, bool right_value);
bool apply_negation   (bool left_value, bool right_value);
bool apply_xor        (bool left_value, bool right_value);
bool apply_disjunction(bool left_value, bool right_value);
bool apply_conjunction(bool left_value, bool right_value);

enum operators_index {
    IMPLICATION = 0,
    EQUIVALENCE = 1,
    NEGATION    = 2,
    XOR         = 3,
    DISJUNCTION = 4,
    CONJUNCTION = 5
};

const operator_parametrs OPERATORS[] = {[IMPLICATION] = {"→", true,   apply_implication, 2},
                                        [EQUIVALENCE] = {"~", true,   apply_equivalence, 3},
                                        [NEGATION]    = {"¬", false,  apply_negation, 3},
                                        [XOR]         = {"+", true,   apply_xor, 2},
                                        [DISJUNCTION] = {"v", true,   apply_disjunction, 1},
                                        [CONJUNCTION] = {"^", true,   apply_conjunction, 1}};

#endif //OPERATORS_H_INCLUDED

