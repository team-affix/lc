#include "infrastructure/env_pool.hpp"

env* env_pool::make_frame(const val* bound_value, env* parent) {
    return alloc(env{bound_value, parent});
}

env* env_pool::alloc(env e) {
    envs_.push_back(std::move(e));
    return &envs_.back();
}
