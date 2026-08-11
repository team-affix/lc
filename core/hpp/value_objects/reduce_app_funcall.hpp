#ifndef REDUCE_APP_FUNCALL_HPP
#define REDUCE_APP_FUNCALL_HPP

#include <memory>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_app_funcall {
    std::shared_ptr<val>& slot;
    const expr::app* application;
    std::shared_ptr<env> e;
};

#endif
