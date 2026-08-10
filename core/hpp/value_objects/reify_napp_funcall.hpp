#ifndef REIFY_NAPP_FUNCALL_HPP
#define REIFY_NAPP_FUNCALL_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_napp_funcall {
    const expr*& out;
    val::napp neutral;
    uint32_t depth;
};

#endif
