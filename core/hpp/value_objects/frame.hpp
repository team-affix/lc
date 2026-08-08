#ifndef FRAME_HPP
#define FRAME_HPP

#include <variant>
#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_val_frame.hpp"

using frame = std::variant<reduce_whnf_frame, reduce_app_frame, reduce_var_frame,
                           reify_val_frame, reify_clo_frame, reify_napp_frame>;

#endif
