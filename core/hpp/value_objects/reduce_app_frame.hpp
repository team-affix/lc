#ifndef REDUCE_APP_FRAME_HPP
#define REDUCE_APP_FRAME_HPP

#include <memory>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reduce_app_frame {
    std::shared_ptr<val>& slot;
    const expr::app* application;
    std::shared_ptr<env> e;
    std::shared_ptr<val> fun_holder;
};

#endif
