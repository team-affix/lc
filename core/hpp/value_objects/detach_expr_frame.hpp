#ifndef DETACH_EXPR_FRAME_HPP
#define DETACH_EXPR_FRAME_HPP

#include <memory>
#include "value_objects/expr.hpp"

struct detach_expr_frame {
    std::shared_ptr<expr>& out;
    std::shared_ptr<expr> src;
    std::shared_ptr<expr> body;
    std::shared_ptr<expr> fun;
    std::shared_ptr<expr> arg;
};

#endif
