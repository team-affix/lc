#include "value_objects/manifest.hpp"

manifest::manifest(std::shared_ptr<expr>& out, const expr* term)
    : exprs()
    , envs()
    , vals()
    , gc(exprs, vals, envs)
    , lookup()
    , red(vals, vals, envs, lookup)
    , re(exprs, exprs, exprs, vals, envs, vals)
    , proc(red, re)
    , norm(vals)
    , interp(std::in_place, proc, proc, norm.normalize(out, exprs.import(term))) {
}
