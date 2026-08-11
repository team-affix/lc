#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/manifest.hpp"

struct runtime {
    runtime(std::shared_ptr<expr> term, uint64_t gc_interval = 1024);
    ~runtime();
    void step();
    bool done() const;
    std::shared_ptr<expr> output() const;
    std::size_t space_usage() const;

  private:
    std::shared_ptr<expr> norm_out_;
    manifest manifest_;
    uint64_t gc_interval_;
    uint64_t steps_;
};

#endif
