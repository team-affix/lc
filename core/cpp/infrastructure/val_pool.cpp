#include "infrastructure/val_pool.hpp"

val_pool::val_pool() : nodes_() {
}

std::shared_ptr<val> val_pool::make_clo(std::shared_ptr<expr> term,
                                        std::shared_ptr<env> e) {
    return nodes_.make(val{val::clo{std::move(term), std::move(e)}});
}

std::shared_ptr<val> val_pool::make_fvar(uint32_t depth) {
    return nodes_.make(val{val::fvar{depth}});
}

std::shared_ptr<val> val_pool::make_napp(std::shared_ptr<val> head,
                                         std::shared_ptr<expr> arg,
                                         std::shared_ptr<env> arg_env) {
    return nodes_.make(
        val{val::napp{std::move(head), std::move(arg), std::move(arg_env)}});
}

bool val_pool::collect_one() {
    return nodes_.collect_one();
}
