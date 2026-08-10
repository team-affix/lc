#ifndef RUNTIME_HPP
#define RUNTIME_HPP

#include "value_objects/expr.hpp"
#include "value_objects/manifest.hpp"

struct runtime {
    runtime(const expr*& out, const expr* term);
    bool step();
    bool done() const;

  private:
    manifest manifest_;
};

#endif
