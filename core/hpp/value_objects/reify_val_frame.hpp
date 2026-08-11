#ifndef REIFY_VAL_FRAME_HPP
#define REIFY_VAL_FRAME_HPP

#include <cstdint>
#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct reify_val_frame {
    std::shared_ptr<expr>& out;
    std::shared_ptr<val> v;
    uint32_t depth;
};

#endif
