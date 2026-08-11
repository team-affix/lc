#ifndef EXPR_FACTORY_HPP
#define EXPR_FACTORY_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include "value_objects/expr.hpp"

template <typename IAllocExpr> struct expr_factory {
    expr_factory(IAllocExpr& alloc_expr);
    std::shared_ptr<expr> make_var(uint32_t index);
    std::shared_ptr<expr> make_abs(std::shared_ptr<expr> body);
    std::shared_ptr<expr> make_app(std::shared_ptr<expr> fun,
                                   std::shared_ptr<expr> arg);

  private:
    IAllocExpr& alloc_expr_;
};

template <typename IAllocExpr>
expr_factory<IAllocExpr>::expr_factory(IAllocExpr& alloc_expr)
    : alloc_expr_(alloc_expr) {
}

template <typename IAllocExpr>
std::shared_ptr<expr> expr_factory<IAllocExpr>::make_var(uint32_t index) {
    return alloc_expr_.alloc(expr{expr::var{index}});
}

template <typename IAllocExpr>
std::shared_ptr<expr>
expr_factory<IAllocExpr>::make_abs(std::shared_ptr<expr> body) {
    return alloc_expr_.alloc(expr{expr::abs{std::move(body)}});
}

template <typename IAllocExpr>
std::shared_ptr<expr>
expr_factory<IAllocExpr>::make_app(std::shared_ptr<expr> fun,
                                   std::shared_ptr<expr> arg) {
    return alloc_expr_.alloc(expr{expr::app{std::move(fun), std::move(arg)}});
}

#endif
