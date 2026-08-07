#ifndef REIFIER_HPP
#define REIFIER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <optional>
#include <variant>

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeReady, typename IEval>
struct reifier {
    reifier(IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
            IMakeFvar& make_fvar, IMakeReady& make_ready, IEval& eval);
    std::optional<const expr*> reify(const val* v, uint32_t depth,
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
std::optional<const expr*>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeReady, IEval>::reify(
    const val* v, uint32_t depth, uint64_t& reductions_left) {
    if (const val::clo* closure = std::get_if<val::clo>(&v->content)) {
        const val* fresh = make_fvar_.make_fvar(depth);
        const env* extended = make_ready_.make_ready(fresh, closure->captured);
        std::optional<const val*> body_val =
            eval_.eval(closure->body, extended, reductions_left);
        if (!body_val.has_value())
            return std::nullopt;
        std::optional<const expr*> body_nf =
            reify(*body_val, depth + 1, reductions_left);
        if (!body_nf.has_value())
            return std::nullopt;
        return make_abs_.make_abs(*body_nf);
    }
    if (const val::fvar* fresh = std::get_if<val::fvar>(&v->content)) {
        DEBUG_ASSERT(depth > fresh->depth);
        return make_var_.make_var(depth - fresh->depth - 1);
    }
    const val::napp& neutral = std::get<val::napp>(v->content);
    std::optional<const expr*> fun_term =
        reify(neutral.head, depth, reductions_left);
    if (!fun_term.has_value())
        return std::nullopt;
    std::optional<const val*> arg_val =
        eval_.eval(neutral.arg, neutral.arg_env, reductions_left);
    if (!arg_val.has_value())
        return std::nullopt;
    std::optional<const expr*> arg_term =
        reify(*arg_val, depth, reductions_left);
    if (!arg_term.has_value())
        return std::nullopt;
    return make_app_.make_app(*fun_term, *arg_term);
}

#endif
