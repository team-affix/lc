#include "infrastructure/runtime.hpp"

#include "debug_assert.hpp"

runtime::runtime(std::shared_ptr<expr> term, rc_pool<expr>& out_nodes)
    : norm_out_(), out_nodes_(out_nodes),
      manifest_(norm_out_, std::move(term), out_nodes_) {
}

runtime::~runtime() {
    manifest_.interp.reset();
    norm_out_.reset();
}

void runtime::step() {
    manifest_.interp->step();
}

bool runtime::done() const {
    return manifest_.interp->done();
}

std::shared_ptr<expr> runtime::output() const {
    DEBUG_ASSERT(done());
    DEBUG_ASSERT(norm_out_);
    return norm_out_;
}

std::size_t runtime::space_usage() const {
    return manifest_.expr_nodes.space_usage() +
           manifest_.val_nodes.space_usage() +
           manifest_.env_nodes.space_usage() + out_nodes_.space_usage() +
           manifest_.interp->space_usage();
}
