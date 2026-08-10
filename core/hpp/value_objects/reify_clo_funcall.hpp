#ifndef REIFY_CLO_FUNCALL_HPP
#define REIFY_CLO_FUNCALL_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_clo_funcall {
    const expr*& out;
    const val::clo* closure;
    uint32_t depth;
};

#endif
