#ifndef REIFY_VAL_FUNCALL_HPP
#define REIFY_VAL_FUNCALL_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_val_funcall {
    std::shared_ptr<expr>& out;
    std::shared_ptr<val> v;
    uint32_t depth;
};

#endif
