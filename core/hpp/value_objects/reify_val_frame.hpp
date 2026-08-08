#ifndef REIFY_VAL_FRAME_HPP
#define REIFY_VAL_FRAME_HPP

#include <cstdint>
#include "value_objects/expr.hpp"
#include "value_objects/reify_val_stage.hpp"
#include "value_objects/val.hpp"

struct reify_val_frame {
    const expr*& out;
    const val* v;
    uint32_t depth;
    reify_val_stage st;
};

#endif
