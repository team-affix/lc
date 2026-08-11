#ifndef DETACH_EXPR_STAGE_HPP
#define DETACH_EXPR_STAGE_HPP

#include <variant>
#include "value_objects/detach_expr_after_arg_stage.hpp"
#include "value_objects/detach_expr_after_body_stage.hpp"
#include "value_objects/detach_expr_after_fun_stage.hpp"
#include "value_objects/detach_expr_start_stage.hpp"

using detach_expr_stage =
    std::variant<detach_expr_start_stage, detach_expr_after_body_stage,
                 detach_expr_after_fun_stage, detach_expr_after_arg_stage>;

#endif
