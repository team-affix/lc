#ifndef REDUCE_VAR_CONTINUATION_HPP
#define REDUCE_VAR_CONTINUATION_HPP

#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_var_stage.hpp"

struct reduce_var_continuation {
    reduce_var_frame frame;
    reduce_var_stage stage;
};

#endif
