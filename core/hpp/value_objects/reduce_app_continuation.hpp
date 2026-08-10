#ifndef REDUCE_APP_CONTINUATION_HPP
#define REDUCE_APP_CONTINUATION_HPP

#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_app_stage.hpp"

struct reduce_app_continuation {
    reduce_app_frame frame;
    reduce_app_stage stage;
};

#endif
