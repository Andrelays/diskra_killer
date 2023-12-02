#ifndef OPERATORS_H_INCLUDED
#define OPERATORS_H_INCLUDED

#include "differentiator.h"

typedef double     (*calculation_function)     (double,      double);
typedef tree_node *(*differentiation_function) (tree_node *, tree_node *, ssize_t);

struct operator_parametrs {
    const char              *name;
    bool                     is_binary;
    calculation_function     calculate_func;
    size_t                   priority;
};

tree_node *differentiation_add(tree_node *left_node, tree_node *right_node, ssize_t variable_index);

enum operators_index {
    IMPLICATION = 0,
    EQUIVALENCE = 1,
    NEGATION    = 2,
    XOR         = 3,
    DISJUNCTION = 4,
    CONJUNCTION = 5
};

const operator_parametrs OPERATORS[] = {[IMPLICATION] = {"→",  true,  NULL, 2},
                                        [EQUIVALENCE] = {"~", true,  NULL, 3},
                                        [NEGATION]    = {"¬",   false, NULL, 3},
                                        [XOR]         = {"+",   true,  NULL, 2},
                                        [DISJUNCTION] = {"v", true,  NULL, 1},
                                        [CONJUNCTION] = {"^", true,  NULL, 1}};

#endif //OPERATORS_H_INCLUDED

