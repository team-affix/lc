#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include <cstdint>
#include <memory>
#include "debug_assert.hpp"
#include "infrastructure/output_detacher.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/manifest.hpp"

template <typename ICollectGarbage, typename IPrepareOutput> struct runtime {
    runtime(std::shared_ptr<expr> term, uint64_t gc_interval = 1024);
    ~runtime();
    void step();
    bool done() const;
    std::shared_ptr<expr> output() const;

  private:
    std::shared_ptr<expr> norm_out_;
    manifest manifest_;
    ICollectGarbage& collect_garbage_;
    IPrepareOutput prepare_output_;
    uint64_t gc_interval_;
    uint64_t steps_;
};

template <typename ICollectGarbage, typename IPrepareOutput>
runtime<ICollectGarbage, IPrepareOutput>::runtime(std::shared_ptr<expr> term,
                                                  uint64_t gc_interval)
    : norm_out_()
    , manifest_(norm_out_, std::move(term))
    , collect_garbage_(manifest_.gc)
    , prepare_output_()
    , gc_interval_(gc_interval)
    , steps_(0) {
    DEBUG_ASSERT(gc_interval_ != 0);
}

template <typename ICollectGarbage, typename IPrepareOutput>
runtime<ICollectGarbage, IPrepareOutput>::~runtime() {
    manifest_.interp.reset();
    norm_out_.reset();
    collect_garbage_.collect();
}

template <typename ICollectGarbage, typename IPrepareOutput>
void runtime<ICollectGarbage, IPrepareOutput>::step() {
    manifest_.interp->step();
    ++steps_;
    if(steps_ % gc_interval_ == 0)
        collect_garbage_.collect();
}

template <typename ICollectGarbage, typename IPrepareOutput>
bool runtime<ICollectGarbage, IPrepareOutput>::done() const {
    return manifest_.interp->done();
}

template <typename ICollectGarbage, typename IPrepareOutput>
std::shared_ptr<expr>
runtime<ICollectGarbage, IPrepareOutput>::output() const {
    DEBUG_ASSERT(done());
    DEBUG_ASSERT(norm_out_);
    return prepare_output_.prepare(norm_out_);
}

using nbe_runtime = runtime<manifest::garbage_collector_t, output_detacher>;

#endif
