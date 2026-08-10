#ifndef REIFY_CLO_CONTINUATION_HPP
#define REIFY_CLO_CONTINUATION_HPP

#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_clo_stage.hpp"

struct reify_clo_continuation {
    reify_clo_frame frame;
    reify_clo_stage stage;
};

#endif
