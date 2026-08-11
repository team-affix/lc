#ifndef DETACH_EXPR_CONTINUATION_HPP
#define DETACH_EXPR_CONTINUATION_HPP

#include "value_objects/detach_expr_frame.hpp"
#include "value_objects/detach_expr_stage.hpp"

struct detach_expr_continuation {
    detach_expr_frame frame;
    detach_expr_stage stage;
};

#endif
