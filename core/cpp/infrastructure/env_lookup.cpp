#include "infrastructure/env_lookup.hpp"
#include "debug_assert.hpp"

env* env_lookup::lookup(env* e, uint32_t index) {
    DEBUG_ASSERT(e != nullptr);
    if(index == 0)
        return e;
    return lookup(e->parent, index - 1);
}
