#ifndef REDUCE_WHNF_FRAME_HPP
#define REDUCE_WHNF_FRAME_HPP

#include <memory>
#include "value_objects/val.hpp"

struct reduce_whnf_frame {
    std::shared_ptr<val>& slot;
};

#endif
