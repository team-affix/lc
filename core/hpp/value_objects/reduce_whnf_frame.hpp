#ifndef REDUCE_WHNF_FRAME_HPP
#define REDUCE_WHNF_FRAME_HPP

#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/val.hpp"

struct reduce_whnf_frame {
    const val*& slot;
    reduce_whnf_stage st;
};

#endif
