#ifndef REIFY_NAPP_FRAME_HPP
#define REIFY_NAPP_FRAME_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_napp_frame {
    std::shared_ptr<expr>& out;
    val::napp neutral;
    uint32_t depth;
    std::shared_ptr<expr> fun_term;
    std::shared_ptr<expr> arg_term;
    std::shared_ptr<val> head_holder;
    std::shared_ptr<val> arg_holder;
};

#endif
