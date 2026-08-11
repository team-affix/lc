#include "infrastructure/env_lookup.hpp"
#include "debug_assert.hpp"

env* env_lookup::lookup(env* e, uint32_t index) {
    while(true) {
        DEBUG_ASSERT(e != nullptr);
        if(index == 0)
            return e;
        e = e->parent.get();
        --index;
    }
}
