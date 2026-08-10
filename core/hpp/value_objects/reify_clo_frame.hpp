#ifndef REIFY_CLO_FRAME_HPP
#define REIFY_CLO_FRAME_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_clo_frame {
    const expr*& out;
    const val::clo* closure;
    uint32_t depth;
    const expr* body_nf;
    const val* body_holder;
};

#endif
