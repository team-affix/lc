#ifndef REDUCE_APP_STAGE_HPP
#define REDUCE_APP_STAGE_HPP

#include <variant>
#include "value_objects/app_after_body_stage.hpp"
#include "value_objects/app_after_fun_stage.hpp"
#include "value_objects/app_need_fun_stage.hpp"

using reduce_app_stage =
    std::variant<app_need_fun_stage, app_after_fun_stage, app_after_body_stage>;

#endif
