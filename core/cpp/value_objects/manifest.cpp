#include "value_objects/manifest.hpp"

manifest::manifest(std::shared_ptr<expr>& out, std::shared_ptr<expr> term,
                   expr_nodes_t& out_nodes)
    : expr_nodes()
    , val_nodes()
    , env_nodes()
    , exprs(expr_nodes)
    , envs(env_nodes)
    , vals(val_nodes)
    , detacher(out_nodes)
    , entrypoint()
    , lookup()
    , red(vals, vals, envs, lookup)
    , re(exprs, exprs, exprs, vals, envs, vals)
    , proc(red, re, entrypoint, detacher)
    , initial_frame_gen(vals)
    , interp(std::in_place, proc, proc,
             initial_frame_gen.generate_initial_frame(out, std::move(term)))
    , space_usage_calculator(expr_nodes, val_nodes, env_nodes, *interp) {
}
