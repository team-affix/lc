#ifndef VAL_POOL_HPP
#define VAL_POOL_HPP

#include <cstdint>
#include <deque>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct val_pool {
    const val* make_clo(const expr* body, env* e);
    const val* make_fvar(uint32_t depth);
    const val* make_napp(const val* head, const expr* arg, env* arg_env);

private:
    const val* alloc(val v);
    std::deque<val> vals_;
};

#endif
