#ifndef CONTINUATION_HPP
#define CONTINUATION_HPP

#include <variant>
#include "value_objects/detach_expr_continuation.hpp"
#include "value_objects/main_continuation.hpp"
#include "value_objects/reduce_app_continuation.hpp"
#include "value_objects/reduce_var_continuation.hpp"
#include "value_objects/reduce_whnf_continuation.hpp"
#include "value_objects/reify_clo_continuation.hpp"
#include "value_objects/reify_napp_continuation.hpp"
#include "value_objects/reify_val_continuation.hpp"

using continuation =
    std::variant<main_continuation, reduce_whnf_continuation,
                 reduce_app_continuation, reduce_var_continuation,
                 reify_val_continuation, reify_clo_continuation,
                 reify_napp_continuation, detach_expr_continuation>;

#endif
