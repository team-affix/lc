#ifndef OUTPUT_DETACHER_HPP
#define OUTPUT_DETACHER_HPP

#include <memory>
#include "value_objects/expr.hpp"

struct output_detacher {
    std::shared_ptr<expr> prepare(const std::shared_ptr<expr>& e) const;
};

#endif
