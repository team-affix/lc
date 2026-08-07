#ifndef EXPR_POOL_HPP
#define EXPR_POOL_HPP

#include <unordered_set>
#include "value_objects/expr.hpp"
#include "value_objects/expr_hash.hpp"

struct expr_pool {
    expr_pool();
    const expr* make_var(uint32_t index);
    const expr* make_abs(const expr* body);
    const expr* make_app(const expr* fun, const expr* arg);

private:
    const expr* intern(expr&& e);
    std::unordered_set<expr, expr_hash> exprs_;
};

#endif
