#ifndef REDUCE_APP_FUNCALL_HPP
#define REDUCE_APP_FUNCALL_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_app_funcall {
    const val*& slot;
    const expr::app* application;
    env* e;
};

#endif
