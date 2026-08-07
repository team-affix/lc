#ifndef EXPR_POOL_HPP
#define EXPR_POOL_HPP

#include <cstdint>
#include <deque>
#include "value_objects/expr.hpp"

struct expr_pool {
    const expr* make_var(uint32_t index);
    const expr* make_abs(const expr* body);
    const expr* make_app(const expr* fun, const expr* arg);

private:
    const expr* alloc(expr e);
    std::deque<expr> exprs_;
};

#endif
