#ifndef REDUCE_WHNF_CONTINUATION_HPP
#define REDUCE_WHNF_CONTINUATION_HPP

#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"

struct reduce_whnf_continuation {
    reduce_whnf_frame frame;
    reduce_whnf_stage stage;
};

#endif
