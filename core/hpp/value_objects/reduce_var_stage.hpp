#ifndef REDUCE_VAR_STAGE_HPP
#define REDUCE_VAR_STAGE_HPP

#include <variant>
#include "value_objects/var_after_force_stage.hpp"
#include "value_objects/var_start_stage.hpp"

using reduce_var_stage = std::variant<var_start_stage, var_after_force_stage>;

#endif
