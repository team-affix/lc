#ifndef REDUCE_WHNF_STAGE_HPP
#define REDUCE_WHNF_STAGE_HPP

#include <variant>
#include "value_objects/whnf_after_child_stage.hpp"
#include "value_objects/whnf_start_stage.hpp"

using reduce_whnf_stage = std::variant<whnf_start_stage, whnf_after_child_stage>;

#endif
