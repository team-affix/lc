#ifndef ENV_FACTORY_HPP
#define ENV_FACTORY_HPP

#include <memory>
#include <utility>
#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

template <typename IAllocEnv> struct env_factory {
    env_factory(IAllocEnv& alloc_env);
    std::shared_ptr<env> make_env(std::shared_ptr<val> bound_value,
                                  std::shared_ptr<env> parent);

  private:
    IAllocEnv& alloc_env_;
};

template <typename IAllocEnv>
env_factory<IAllocEnv>::env_factory(IAllocEnv& alloc_env)
    : alloc_env_(alloc_env) {
}

template <typename IAllocEnv>
std::shared_ptr<env>
env_factory<IAllocEnv>::make_env(std::shared_ptr<val> bound_value,
                                 std::shared_ptr<env> parent) {
    return alloc_env_.alloc(env{std::move(bound_value), std::move(parent)});
}

#endif
