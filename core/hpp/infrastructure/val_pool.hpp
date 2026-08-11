#ifndef VAL_POOL_HPP
#define VAL_POOL_HPP

#include <cstdint>
#include <memory>
#include "infrastructure/rc_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct val_pool {
    val_pool();
    std::shared_ptr<val> make_clo(std::shared_ptr<expr> term,
                                  std::shared_ptr<env> e);
    std::shared_ptr<val> make_fvar(uint32_t depth);
    std::shared_ptr<val> make_napp(std::shared_ptr<val> head,
                                   std::shared_ptr<expr> arg,
                                   std::shared_ptr<env> arg_env);

  private:
    rc_pool<val> nodes_;
};

#endif
