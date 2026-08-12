#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <cstddef>
#include <memory>
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/manifest.hpp"

struct runtime {
    runtime(std::shared_ptr<expr> term, rc_pool<expr>& out_nodes);
    ~runtime();
    void step();
    bool done() const;
    std::shared_ptr<expr> output() const;
    std::size_t space_usage() const;

  private:
    std::shared_ptr<expr> norm_out_;
    rc_pool<expr>& out_nodes_;
    manifest manifest_;
};

#endif
