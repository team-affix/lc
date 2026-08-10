#ifndef REIFY_NAPP_FRAME_HPP
#define REIFY_NAPP_FRAME_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_napp_frame {
    const expr*& out;
    val::napp neutral;
    uint32_t depth;
    const expr* fun_term;
    const expr* arg_term;
    const val* head_holder;
    const val* arg_holder;
};

#endif
