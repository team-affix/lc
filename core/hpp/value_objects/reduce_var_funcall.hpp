#ifndef REDUCE_VAR_FUNCALL_HPP
#define REDUCE_VAR_FUNCALL_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_var_funcall {
    const val*& slot;
    const expr::var* variable;
    env* e;
};

#endif
