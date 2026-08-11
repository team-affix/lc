#include "infrastructure/expr_pool.hpp"
#include <variant>

expr_pool::expr_pool() : nodes_() {
}

std::shared_ptr<expr> expr_pool::make_var(uint32_t index) {
    return nodes_.make(expr{expr::var{index}});
}

std::shared_ptr<expr> expr_pool::make_abs(std::shared_ptr<expr> body) {
    return nodes_.make(expr{expr::abs{std::move(body)}});
}

std::shared_ptr<expr> expr_pool::make_app(std::shared_ptr<expr> fun,
                                          std::shared_ptr<expr> arg) {
    return nodes_.make(expr{expr::app{std::move(fun), std::move(arg)}});
}

std::shared_ptr<expr> expr_pool::import(const std::shared_ptr<expr>& e) {
    return import(e.get());
}

std::shared_ptr<expr> expr_pool::import(const expr* e) {
    if(const expr::var* v = std::get_if<expr::var>(&e->content))
        return make_var(v->index);
    if(const expr::abs* a = std::get_if<expr::abs>(&e->content))
        return make_abs(import(a->body));
    const expr::app& ap = std::get<expr::app>(e->content);
    return make_app(import(ap.fun), import(ap.arg));
}
