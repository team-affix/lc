#include "infrastructure/runtime.hpp"

runtime::runtime(const expr*& out, const expr* term) : manifest_(out, term) {
}

bool runtime::step() {
    return manifest_.interp.step();
}

bool runtime::done() const {
    return manifest_.interp.done();
}
