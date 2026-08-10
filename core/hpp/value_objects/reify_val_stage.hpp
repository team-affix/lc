#ifndef REIFY_VAL_STAGE_HPP
#define REIFY_VAL_STAGE_HPP

#include <variant>
#include "value_objects/reify_val_after_quote_stage.hpp"
#include "value_objects/reify_val_after_whnf_stage.hpp"
#include "value_objects/reify_val_need_whnf_stage.hpp"

using reify_val_stage =
    std::variant<reify_val_need_whnf_stage, reify_val_after_whnf_stage,
                 reify_val_after_quote_stage>;

#endif
