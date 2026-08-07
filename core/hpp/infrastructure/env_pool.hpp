#ifndef ENV_POOL_HPP
#define ENV_POOL_HPP

#include <deque>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct env_pool {
    env_pool();
    const env* make_delayed(const expr* arg, const env* arg_env, const env* parent);
    const env* make_ready(const val* value, const env* parent);

private:
    const env* alloc(env e);
    std::deque<env> envs_;
};

#endif
