#ifndef REIFY_NAPP_FUNCALL_HPP
#define REIFY_NAPP_FUNCALL_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_napp_funcall {
    std::shared_ptr<expr>& out;
    val::napp neutral;
    uint32_t depth;
};

#endif
