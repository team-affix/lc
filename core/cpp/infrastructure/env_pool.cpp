#include "infrastructure/env_pool.hpp"

env_pool::env_pool() : envs_() {}

const env* env_pool::make_delayed(const expr* arg,
                                  const env* arg_env,
                                  const env* parent) {
    envs_.push_back(env{env::delayed{arg, arg_env}, parent});
    return &envs_.back();
}

const env* env_pool::make_ready(const val* value, const env* parent) {
    envs_.push_back(env{env::ready{value}, parent});
    return &envs_.back();
}
