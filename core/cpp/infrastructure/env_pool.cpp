#include "infrastructure/env_pool.hpp"

const env* env_pool::make_delayed(const expr* arg,
                                  const env* arg_env,
                                  const env* parent) {
    return alloc(env{env::delayed{arg, arg_env}, parent});
}

const env* env_pool::make_ready(const val* value, const env* parent) {
    return alloc(env{env::ready{value}, parent});
}

const env* env_pool::alloc(env e) {
    envs_.push_back(std::move(e));
    return &envs_.back();
}
