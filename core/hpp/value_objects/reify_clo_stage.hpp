#ifndef REIFY_CLO_STAGE_HPP
#define REIFY_CLO_STAGE_HPP

#include <variant>
#include "value_objects/reify_clo_after_body_stage.hpp"
#include "value_objects/reify_clo_need_body_stage.hpp"

using reify_clo_stage =
    std::variant<reify_clo_need_body_stage, reify_clo_after_body_stage>;

#endif
