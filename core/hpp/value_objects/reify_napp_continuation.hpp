#ifndef REIFY_NAPP_CONTINUATION_HPP
#define REIFY_NAPP_CONTINUATION_HPP

#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_napp_stage.hpp"

struct reify_napp_continuation {
    reify_napp_frame frame;
    reify_napp_stage stage;
};

#endif
