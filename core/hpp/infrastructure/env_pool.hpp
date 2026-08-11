#ifndef ENV_POOL_HPP
#define ENV_POOL_HPP

#include <memory>
#include "infrastructure/rc_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

struct env_pool {
    env_pool();
    std::shared_ptr<env> make_env(std::shared_ptr<val> bound_value,
                                  std::shared_ptr<env> parent);
    bool collect_one();

  private:
    rc_pool<env> nodes_;
};

#endif
