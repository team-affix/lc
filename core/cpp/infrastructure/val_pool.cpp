#include "infrastructure/val_pool.hpp"

val_pool::val_pool() : vals_() {}

const val* val_pool::make_clo(const expr* body, const env* e) {
    vals_.push_back(val{val::clo{body, e}});
    return &vals_.back();
}

const val* val_pool::make_fvar(uint32_t depth) {
    vals_.push_back(val{val::fvar{depth}});
    return &vals_.back();
}

const val* val_pool::make_napp(const val* head, const expr* arg, const env* arg_env) {
    vals_.push_back(val{val::napp{head, arg, arg_env}});
    return &vals_.back();
}
