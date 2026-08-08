#ifndef EXPRS_EQ_HPP
#define EXPRS_EQ_HPP

#include "value_objects/expr.hpp"
#include <utility>
#include <variant>
#include <vector>

inline bool exprs_eq(const expr* a, const expr* b) {
    std::vector<std::pair<const expr*, const expr*>> stack;
    stack.emplace_back(a, b);
    while(!stack.empty()) {
        const expr* l = stack.back().first;
        const expr* r = stack.back().second;
        stack.pop_back();
        if(l == r)
            continue;
        if(l == nullptr || r == nullptr)
            return false;
        if(l->content.index() != r->content.index())
            return false;
        if(const expr::var* lv = std::get_if<expr::var>(&l->content)) {
            if(lv->index != std::get<expr::var>(r->content).index)
                return false;
            continue;
        }
        if(const expr::abs* la = std::get_if<expr::abs>(&l->content)) {
            stack.emplace_back(la->body, std::get<expr::abs>(r->content).body);
            continue;
        }
        const expr::app& la = std::get<expr::app>(l->content);
        const expr::app& ra = std::get<expr::app>(r->content);
        stack.emplace_back(la.arg, ra.arg);
        stack.emplace_back(la.fun, ra.fun);
    }
    return true;
}

#endif
