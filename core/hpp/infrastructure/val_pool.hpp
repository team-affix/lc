#ifndef VAL_POOL_HPP
#define VAL_POOL_HPP

#include <cstdint>
#include <memory>
#include <utility>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

template <typename IAllocVal> struct val_pool {
    val_pool(IAllocVal& alloc_val);
    std::shared_ptr<val> make_clo(std::shared_ptr<expr> term,
                                  std::shared_ptr<env> e);
    std::shared_ptr<val> make_fvar(uint32_t depth);
    std::shared_ptr<val> make_napp(std::shared_ptr<val> head,
                                   std::shared_ptr<expr> arg,
                                   std::shared_ptr<env> arg_env);

  private:
    IAllocVal& alloc_val_;
};

template <typename IAllocVal>
val_pool<IAllocVal>::val_pool(IAllocVal& alloc_val) : alloc_val_(alloc_val) {
}

template <typename IAllocVal>
std::shared_ptr<val> val_pool<IAllocVal>::make_clo(std::shared_ptr<expr> term,
                                                   std::shared_ptr<env> e) {
    return alloc_val_.alloc(val{val::clo{std::move(term), std::move(e)}});
}

template <typename IAllocVal>
std::shared_ptr<val> val_pool<IAllocVal>::make_fvar(uint32_t depth) {
    return alloc_val_.alloc(val{val::fvar{depth}});
}

template <typename IAllocVal>
std::shared_ptr<val>
val_pool<IAllocVal>::make_napp(std::shared_ptr<val> head,
                               std::shared_ptr<expr> arg,
                               std::shared_ptr<env> arg_env) {
    return alloc_val_.alloc(
        val{val::napp{std::move(head), std::move(arg), std::move(arg_env)}});
}

#endif
