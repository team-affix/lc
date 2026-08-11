#ifndef MAIN_FUNCALL_HPP
#define MAIN_FUNCALL_HPP

#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct main_funcall {
    std::shared_ptr<expr>& detached_out;
    std::shared_ptr<val> seed;
};

#endif
