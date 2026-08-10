#include "infrastructure/runtime.hpp"

runtime::runtime(const expr*& out, const expr* term) : manifest_(out, term) {
}

void runtime::step() {
    manifest_.interp.step();
}

bool runtime::done() const {
    return manifest_.interp.done();
}
