#ifndef MAIN_STAGE_HPP
#define MAIN_STAGE_HPP

#include <variant>
#include "value_objects/main_after_detach_stage.hpp"
#include "value_objects/main_detach_stage.hpp"
#include "value_objects/main_reify_stage.hpp"

using main_stage = std::variant<main_reify_stage, main_detach_stage,
                                main_after_detach_stage>;

#endif
