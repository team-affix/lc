#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/manifest.hpp"

template <typename ICollectGarbage> struct runtime {
    runtime(std::shared_ptr<expr>& out, const expr* term);
    ~runtime();
    void step();
    bool done() const;

  private:
    static constexpr uint64_t k_gc_interval = 1024;

    manifest manifest_;
    ICollectGarbage& collect_garbage_;
    uint64_t steps_;
};

template <typename ICollectGarbage>
runtime<ICollectGarbage>::runtime(std::shared_ptr<expr>& out, const expr* term)
    : manifest_(out, term), collect_garbage_(manifest_.gc), steps_(0) {
}

template <typename ICollectGarbage> runtime<ICollectGarbage>::~runtime() {
    manifest_.interp.reset();
    collect_garbage_.collect();
}

template <typename ICollectGarbage> void runtime<ICollectGarbage>::step() {
    manifest_.interp->step();
    ++steps_;
    if(steps_ % k_gc_interval == 0)
        collect_garbage_.collect();
}

template <typename ICollectGarbage> bool runtime<ICollectGarbage>::done() const {
    return manifest_.interp->done();
}

using nbe_runtime = runtime<manifest::garbage_collector_t>;

#endif
