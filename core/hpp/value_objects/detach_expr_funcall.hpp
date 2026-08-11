#ifndef DETACH_EXPR_FUNCALL_HPP
#define DETACH_EXPR_FUNCALL_HPP

#include <memory>
#include "value_objects/expr.hpp"

struct detach_expr_funcall {
    std::shared_ptr<expr>& out;
    std::shared_ptr<expr> src;
};

#endif
