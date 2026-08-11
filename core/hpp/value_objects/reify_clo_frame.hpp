#ifndef REIFY_CLO_FRAME_HPP
#define REIFY_CLO_FRAME_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_clo_frame {
    std::shared_ptr<expr>& out;
    const val::clo* closure;
    uint32_t depth;
    std::shared_ptr<expr> body_nf;
    std::shared_ptr<val> body_holder;
};

#endif
