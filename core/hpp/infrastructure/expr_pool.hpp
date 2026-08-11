#ifndef EXPR_POOL_HPP
#define EXPR_POOL_HPP

#include <cstdint>
#include <memory>
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"

struct expr_pool {
    expr_pool();
    std::shared_ptr<expr> make_var(uint32_t index);
    std::shared_ptr<expr> make_abs(std::shared_ptr<expr> body);
    std::shared_ptr<expr> make_app(std::shared_ptr<expr> fun,
                                   std::shared_ptr<expr> arg);
    std::shared_ptr<expr> import(const expr* e);
    std::shared_ptr<expr> import(const std::shared_ptr<expr>& e);
    bool collect_one();

  private:
    rc_pool<expr> nodes_;
};

#endif
