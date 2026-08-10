#ifndef REDUCE_VAR_FRAME_HPP
#define REDUCE_VAR_FRAME_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_var_frame {
    const val*& slot;
    const expr::var* variable;
    env* e;
    env* cell;
};

#endif
