#include "infrastructure/val_pool.hpp"

val_pool::val_pool() : vals_() {}

const val* val_pool::make_clo(const expr* body, const env* e) {
    return alloc(val{val::clo{body, e}});
}

const val* val_pool::make_fvar(uint32_t depth) {
    return alloc(val{val::fvar{depth}});
}

const val* val_pool::make_napp(const val* head, const expr* arg, const env* arg_env) {
    return alloc(val{val::napp{head, arg, arg_env}});
}

const val* val_pool::alloc(val v) {
    vals_.push_back(std::move(v));
    return &vals_.back();
}
