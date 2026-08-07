#include "infrastructure/env_pool.hpp"

env* env_pool::make_delayed(const expr* arg, env* arg_env, env* parent) {
    return alloc(env{env::delayed{arg, arg_env}, parent});
}

env* env_pool::make_ready(const val* value, env* parent) {
    return alloc(env{env::ready{value}, parent});
}

env* env_pool::alloc(env e) {
    envs_.push_back(std::move(e));
    return &envs_.back();
}
