#ifndef REDUCE_VAR_FUNCALL_HPP
#define REDUCE_VAR_FUNCALL_HPP

#include <memory>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_var_funcall {
    std::shared_ptr<val>& slot;
    const expr::var* variable;
    std::shared_ptr<env> e;
};

#endif
