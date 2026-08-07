#ifndef EXPRS_EQ_HPP
#define EXPRS_EQ_HPP

#include "value_objects/expr.hpp"
#include <variant>

inline bool exprs_eq(const expr* a, const expr* b) {
    if (a == b)
        return true;
    if (a == nullptr || b == nullptr)
        return false;
    if (a->content.index() != b->content.index())
        return false;
    if (const expr::var* va = std::get_if<expr::var>(&a->content))
        return va->index == std::get<expr::var>(b->content).index;
    if (const expr::abs* aa = std::get_if<expr::abs>(&a->content))
        return exprs_eq(aa->body, std::get<expr::abs>(b->content).body);
    const expr::app& appa = std::get<expr::app>(a->content);
    const expr::app& appb = std::get<expr::app>(b->content);
    return exprs_eq(appa.fun, appb.fun) && exprs_eq(appa.arg, appb.arg);
}

#endif
