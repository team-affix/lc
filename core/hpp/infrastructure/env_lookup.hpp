#ifndef ENV_LOOKUP_HPP
#define ENV_LOOKUP_HPP

#include <cstdint>
#include "value_objects/env.hpp"

struct env_lookup {
    env* lookup(env* e, uint32_t index);
};

#endif
