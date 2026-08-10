#ifndef REIFY_VAL_CONTINUATION_HPP
#define REIFY_VAL_CONTINUATION_HPP

#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_stage.hpp"

struct reify_val_continuation {
    reify_val_frame frame;
    reify_val_stage stage;
};

#endif
