#ifndef MAIN_CONTINUATION_HPP
#define MAIN_CONTINUATION_HPP

#include "value_objects/main_frame.hpp"
#include "value_objects/main_stage.hpp"

struct main_continuation {
    main_frame frame;
    main_stage stage;
};

#endif
