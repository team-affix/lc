#include "value_objects/manifest.hpp"

manifest::manifest(const expr*& out, const expr* term)
    : exprs()
    , envs()
    , vals()
    , lookup()
    , red(vals, vals, envs, lookup)
    , re(exprs, exprs, exprs, vals, envs, vals)
    , proc(red, re)
    , norm(vals)
    , interp(proc, proc, norm.normalize(out, term)) {
}
