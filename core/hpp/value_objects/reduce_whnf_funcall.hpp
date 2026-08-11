#ifndef REDUCE_WHNF_FUNCALL_HPP
#define REDUCE_WHNF_FUNCALL_HPP

#include <memory>
#include "value_objects/val.hpp"

struct reduce_whnf_funcall {
    std::shared_ptr<val>& slot;
};

#endif
