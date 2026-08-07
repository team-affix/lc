#include "infrastructure/expr_pool.hpp"

expr_pool::expr_pool() : exprs_() {}

const expr* expr_pool::make_var(uint32_t index) {
    return intern(expr{expr::var{index}});
}

const expr* expr_pool::make_abs(const expr* body) {
    return intern(expr{expr::abs{body}});
}

const expr* expr_pool::make_app(const expr* fun, const expr* arg) {
    return intern(expr{expr::app{fun, arg}});
}

const expr* expr_pool::intern(expr&& e) {
    return &*exprs_.insert(std::move(e)).first;
}
