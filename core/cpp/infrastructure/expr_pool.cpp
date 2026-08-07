#include "infrastructure/expr_pool.hpp"

const expr* expr_pool::make_var(uint32_t index) {
    return alloc(expr{expr::var{index}});
}

const expr* expr_pool::make_abs(const expr* body) {
    return alloc(expr{expr::abs{body}});
}

const expr* expr_pool::make_app(const expr* fun, const expr* arg) {
    return alloc(expr{expr::app{fun, arg}});
}

const expr* expr_pool::alloc(expr e) {
    exprs_.push_back(std::move(e));
    return &exprs_.back();
}
