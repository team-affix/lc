#ifndef REDUCE_APP_FRAME_HPP
#define REDUCE_APP_FRAME_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/reduce_app_stage.hpp"
#include "value_objects/val.hpp"

struct reduce_app_frame {
    const val*& slot;
    const expr::app* application;
    env* e;
    const val* fun_holder;
    reduce_app_stage st;
};

#endif
