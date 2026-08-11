#ifndef OUTPUT_DETACHER_HPP
#define OUTPUT_DETACHER_HPP

#include <memory>
#include <optional>
#include <utility>
#include "value_objects/detach_expr_after_arg_stage.hpp"
#include "value_objects/detach_expr_after_body_stage.hpp"
#include "value_objects/detach_expr_after_fun_stage.hpp"
#include "value_objects/detach_expr_continuation.hpp"
#include "value_objects/detach_expr_frame.hpp"
#include "value_objects/detach_expr_funcall.hpp"
#include "value_objects/detach_expr_stage.hpp"
#include "value_objects/detach_expr_start_stage.hpp"
#include "value_objects/expr.hpp"

struct output_detacher {
    detach_expr_continuation init_continuation(detach_expr_funcall fc);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_start_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_body_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_fun_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_arg_stage);
};

#endif
