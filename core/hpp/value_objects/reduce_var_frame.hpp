#ifndef REDUCE_VAR_FRAME_HPP
#define REDUCE_VAR_FRAME_HPP

#include <memory>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_var_frame {
    std::shared_ptr<val>& slot;
    const expr::var* variable;
    std::shared_ptr<env> e;
    env* cell;
};

#endif
