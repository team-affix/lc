#ifndef REIFIER_HPP
#define REIFIER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <variant>

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeReady, typename IEval>
struct reifier {
    reifier(IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
            IMakeFvar& make_fvar, IMakeReady& make_ready, IEval& eval);
    bool reify(const expr*& out, const val* v, uint32_t depth,
               uint64_t& reductions_left);

private:
    IMakeVar& make_var_;
    IMakeAbs& make_abs_;
    IMakeApp& make_app_;
    IMakeFvar& make_fvar_;
    IMakeReady& make_ready_;
    IEval& eval_;
};

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeReady, typename IEval>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeReady, IEval>::reifier(
    IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
    IMakeFvar& make_fvar, IMakeReady& make_ready, IEval& eval)
    : make_var_(make_var)
    , make_abs_(make_abs)
    , make_app_(make_app)
    , make_fvar_(make_fvar)
    , make_ready_(make_ready)
    , eval_(eval) {}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeReady, typename IEval>
bool reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeReady, IEval>::reify(
    const expr*& out, const val* v, uint32_t depth,
    uint64_t& reductions_left) {
    if (const val::clo* closure = std::get_if<val::clo>(&v->content)) {
        const val* fresh = make_fvar_.make_fvar(depth);
        env* extended = make_ready_.make_ready(fresh, closure->captured);
        const val* body_val;
        if (!eval_.eval(body_val, closure->body, extended, reductions_left))
            return false;
        const expr* body_nf;
        if (!reify(body_nf, body_val, depth + 1, reductions_left))
            return false;
        out = make_abs_.make_abs(body_nf);
        return true;
    }
    if (const val::fvar* fresh = std::get_if<val::fvar>(&v->content)) {
        DEBUG_ASSERT(depth > fresh->depth);
        out = make_var_.make_var(depth - fresh->depth - 1);
        return true;
    }
    const val::napp& neutral = std::get<val::napp>(v->content);
    const expr* fun_term;
    if (!reify(fun_term, neutral.head, depth, reductions_left))
        return false;
    const val* arg_val;
    if (!eval_.eval(arg_val, neutral.arg, neutral.arg_env, reductions_left))
        return false;
    const expr* arg_term;
    if (!reify(arg_term, arg_val, depth, reductions_left))
        return false;
    out = make_app_.make_app(fun_term, arg_term);
    return true;
}

#endif
