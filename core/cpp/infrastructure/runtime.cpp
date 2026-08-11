#include "infrastructure/runtime.hpp"

#include "debug_assert.hpp"

runtime::runtime(std::shared_ptr<expr> term, uint64_t gc_interval)
    : norm_out_(), manifest_(norm_out_, std::move(term)),
      gc_interval_(gc_interval), steps_(0) {
}

runtime::~runtime() {
    manifest_.interp.reset();
    norm_out_.reset();
    manifest_.gc.collect();
}

void runtime::step() {
    manifest_.interp->step();
    ++steps_;
    if(steps_ % gc_interval_ == 0)
        manifest_.gc.collect();
}

bool runtime::done() const {
    return manifest_.interp->done();
}

std::shared_ptr<expr> runtime::output() const {
    DEBUG_ASSERT(done());
    DEBUG_ASSERT(norm_out_);
    return manifest_.detacher.prepare(norm_out_);
}

std::size_t runtime::space_usage() const {
    return manifest_.expr_nodes.space_usage() +
           manifest_.val_nodes.space_usage() +
           manifest_.env_nodes.space_usage() + manifest_.interp->space_usage();
}
