#ifndef REIFY_VAL_FUNCALL_HPP
#define REIFY_VAL_FUNCALL_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_val_funcall {
    const expr*& out;
    const val* v;
    uint32_t depth;
};

#endif
