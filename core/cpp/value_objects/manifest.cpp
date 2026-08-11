#include "value_objects/manifest.hpp"

manifest::manifest(std::shared_ptr<expr>& out, const expr* term)
    : expr_nodes()
    , val_nodes()
    , env_nodes()
    , exprs(expr_nodes)
    , envs(env_nodes)
    , vals(val_nodes)
    , gc(expr_nodes, val_nodes, env_nodes)
    , lookup()
    , red(vals, vals, envs, lookup)
    , re(exprs, exprs, exprs, vals, envs, vals)
    , proc(red, re)
    , norm(vals)
    , interp(std::in_place, proc, proc,
             norm.normalize(out, exprs.import(term))) {
}
