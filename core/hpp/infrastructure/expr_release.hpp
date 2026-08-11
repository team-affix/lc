#ifndef EXPR_RELEASE_HPP
#define EXPR_RELEASE_HPP

#include <memory>
#include <utility>
#include <variant>
#include <vector>
#include "value_objects/expr.hpp"

// Break a uniquely owned expr tree without O(depth) C++ stack use.
// Shared (use_count > 1) nodes are left alone aside from dropping our ref.
inline void release_expr_iterative(std::shared_ptr<expr>& root) {
    std::vector<std::shared_ptr<expr>> stack;
    if(root)
        stack.push_back(std::move(root));
    while(!stack.empty()) {
        std::shared_ptr<expr> cur = std::move(stack.back());
        stack.pop_back();
        if(!cur)
            continue;
        if(cur.use_count() != 1)
            continue;
        if(expr::abs* a = std::get_if<expr::abs>(&cur->content)) {
            if(a->body)
                stack.push_back(std::move(a->body));
        } else if(expr::app* ap = std::get_if<expr::app>(&cur->content)) {
            if(ap->fun)
                stack.push_back(std::move(ap->fun));
            if(ap->arg)
                stack.push_back(std::move(ap->arg));
        }
        cur->content = expr::var{0};
    }
}

struct expr_tree_owner {
    std::shared_ptr<expr> root;

    expr_tree_owner(std::shared_ptr<expr> r);
    ~expr_tree_owner();
};

inline expr_tree_owner::expr_tree_owner(std::shared_ptr<expr> r)
    : root(std::move(r)) {
}

inline expr_tree_owner::~expr_tree_owner() {
    release_expr_iterative(root);
}

// Root shared_ptr whose last release tears the tree down iteratively.
inline std::shared_ptr<expr> adopt_expr_tree(std::shared_ptr<expr> root) {
    if(!root)
        return root;
    std::shared_ptr<expr_tree_owner> owner =
        std::make_shared<expr_tree_owner>(std::move(root));
    return std::shared_ptr<expr>(owner, owner->root.get());
}

#endif
