#ifndef REIFY_NAPP_STAGE_HPP
#define REIFY_NAPP_STAGE_HPP

#include <variant>
#include "value_objects/reify_napp_after_arg_stage.hpp"
#include "value_objects/reify_napp_need_arg_stage.hpp"
#include "value_objects/reify_napp_need_head_stage.hpp"

using reify_napp_stage =
    std::variant<reify_napp_need_head_stage, reify_napp_need_arg_stage,
                 reify_napp_after_arg_stage>;

#endif
