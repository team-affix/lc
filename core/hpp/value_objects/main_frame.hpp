#ifndef MAIN_FRAME_HPP
#define MAIN_FRAME_HPP

#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct main_frame {
    std::shared_ptr<expr>& detached_out;
    std::shared_ptr<expr> pool_out;
    std::shared_ptr<val> seed;
};

#endif
