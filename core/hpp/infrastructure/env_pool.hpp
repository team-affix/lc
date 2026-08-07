#ifndef ENV_POOL_HPP
#define ENV_POOL_HPP

#include <deque>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct env_pool {
    env* make_delayed(const expr* arg, env* arg_env, env* parent);
    env* make_ready(const val* value, env* parent);

private:
    env* alloc(env e);
    std::deque<env> envs_;
};

#endif
