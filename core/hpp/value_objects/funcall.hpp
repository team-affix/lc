#ifndef FUNCALL_HPP
#define FUNCALL_HPP

#include <variant>
#include "value_objects/reduce_app_funcall.hpp"
#include "value_objects/reduce_var_funcall.hpp"
#include "value_objects/reduce_whnf_funcall.hpp"
#include "value_objects/reify_clo_funcall.hpp"
#include "value_objects/reify_napp_funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"

using funcall =
    std::variant<reduce_whnf_funcall, reduce_app_funcall, reduce_var_funcall,
                 reify_val_funcall, reify_clo_funcall, reify_napp_funcall>;

#endif
