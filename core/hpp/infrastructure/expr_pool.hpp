#ifndef EXPR_POOL_HPP
#define EXPR_POOL_HPP

#include "value_objects/expr.hpp"
#include <cstdint>
#include <memory>
#include <utility>
#include <variant>

template <typename IAllocExpr> struct expr_pool {
    expr_pool(IAllocExpr& alloc_expr);
    std::shared_ptr<expr> make_var(uint32_t index);
    std::shared_ptr<expr> make_abs(std::shared_ptr<expr> body);
    std::shared_ptr<expr> make_app(std::shared_ptr<expr> fun,
                                   std::shared_ptr<expr> arg);
    std::shared_ptr<expr> import(const std::shared_ptr<expr>& e);

  private:
    IAllocExpr& alloc_expr_;
};

template <typename IAllocExpr>
expr_pool<IAllocExpr>::expr_pool(IAllocExpr& alloc_expr)
    : alloc_expr_(alloc_expr) {
}

template <typename IAllocExpr>
std::shared_ptr<expr> expr_pool<IAllocExpr>::make_var(uint32_t index) {
    return alloc_expr_.alloc(expr{expr::var{index}});
}

template <typename IAllocExpr>
std::shared_ptr<expr>
expr_pool<IAllocExpr>::make_abs(std::shared_ptr<expr> body) {
    return alloc_expr_.alloc(expr{expr::abs{std::move(body)}});
}

template <typename IAllocExpr>
std::shared_ptr<expr>
expr_pool<IAllocExpr>::make_app(std::shared_ptr<expr> fun,
                                std::shared_ptr<expr> arg) {
    return alloc_expr_.alloc(expr{expr::app{std::move(fun), std::move(arg)}});
}

template <typename IAllocExpr>
std::shared_ptr<expr>
expr_pool<IAllocExpr>::import(const std::shared_ptr<expr>& e) {
    if(const expr::var* v = std::get_if<expr::var>(&e->content))
        return make_var(v->index);
    if(const expr::abs* a = std::get_if<expr::abs>(&e->content))
        return make_abs(import(a->body));
    const expr::app& ap = std::get<expr::app>(e->content);
    return make_app(import(ap.fun), import(ap.arg));
}

#endif
