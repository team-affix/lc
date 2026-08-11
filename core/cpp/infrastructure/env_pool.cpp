#include "infrastructure/env_pool.hpp"

env_pool::env_pool() : nodes_() {
}

std::shared_ptr<env> env_pool::make_env(std::shared_ptr<val> bound_value,
                                        std::shared_ptr<env> parent) {
    return nodes_.make(env{std::move(bound_value), std::move(parent)});
}
