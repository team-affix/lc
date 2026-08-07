#ifndef REDUCER_HPP
#define REDUCER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <variant>

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
struct reducer {
    reducer(IMakeClo& make_clo, IMakeNapp& make_napp, IMakeFrame& make_frame,
            ILookup& lookup);
    bool whnf(const val*& out, const val* v, uint64_t& reductions_left);

  private:
    bool whnf_clo(const val*& out, const val* v, uint64_t& reductions_left);
    bool whnf_fvar(const val*& out, const val* v, uint64_t& reductions_left);
    bool whnf_napp(const val*& out, const val* v, uint64_t& reductions_left);

    bool whnf_var(const val*& out, const val* v, uint64_t& reductions_left);
    bool whnf_abs(const val*& out, const val* v, uint64_t& reductions_left);
    bool whnf_app(const val*& out, const val* v, uint64_t& reductions_left);

    IMakeClo& make_clo_;
    IMakeNapp& make_napp_;
    IMakeFrame& make_frame_;
    ILookup& lookup_;
};

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::reducer(
    IMakeClo& make_clo, IMakeNapp& make_napp, IMakeFrame& make_frame,
    ILookup& lookup)
    : make_clo_(make_clo), make_napp_(make_napp), make_frame_(make_frame),
      lookup_(lookup) {
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf(
    const val*& out, const val* v, uint64_t& reductions_left) {
    if(std::holds_alternative<val::clo>(v->content))
        return whnf_clo(out, v, reductions_left);
    if(std::holds_alternative<val::fvar>(v->content))
        return whnf_fvar(out, v, reductions_left);
    return whnf_napp(out, v, reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_clo(
    const val*& out, const val* v, uint64_t& reductions_left) {
    const val::clo& closure = std::get<val::clo>(v->content);
    const expr* term = closure.term;
    if(std::holds_alternative<expr::var>(term->content))
        return whnf_var(out, v, reductions_left);
    if(std::holds_alternative<expr::abs>(term->content))
        return whnf_abs(out, v, reductions_left);
    return whnf_app(out, v, reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_fvar(
    const val*& out, const val* v, uint64_t&) {
    out = v;
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_napp(
    const val*& out, const val* v, uint64_t&) {
    out = v;
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_var(
    const val*& out, const val* v, uint64_t& reductions_left) {
    const val::clo& closure = std::get<val::clo>(v->content);
    const expr::var& variable = std::get<expr::var>(closure.term->content);

    ///////////
    //// LOOKUP
    ///////////
    env* cell = lookup_.lookup(closure.environment, variable.index);

    ///////////
    //// WHNF + OVERWRITE
    ///////////
    if(!whnf(out, cell->bound_value, reductions_left))
        return false;
    cell->bound_value = out;
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_abs(
    const val*& out, const val* v, uint64_t&) {
    out = v;
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_app(
    const val*& out, const val* v, uint64_t& reductions_left) {
    const val::clo& closure = std::get<val::clo>(v->content);
    const expr::app& application = std::get<expr::app>(closure.term->content);
    env* e = closure.environment;

    ///////////
    //// WHNF FUN
    ///////////
    const val* fun_val;
    if(!whnf(fun_val, make_clo_.make_clo(application.fun, e), reductions_left))
        return false;
    const val::clo* fun_clo = std::get_if<val::clo>(&fun_val->content);

    ///////////
    //// APPLY NEUTRAL
    ///////////
    if(fun_clo == nullptr) {
        out = make_napp_.make_napp(fun_val, application.arg, e);
        return true;
    }

    ///////////
    //// APPLY CLO (BETA)
    ///////////
    DEBUG_ASSERT(std::holds_alternative<expr::abs>(fun_clo->term->content));
    if(reductions_left == 0)
        return false;
    --reductions_left;
    const expr* body = std::get<expr::abs>(fun_clo->term->content).body;
    env* extended = make_frame_.make_frame(
        make_clo_.make_clo(application.arg, e), fun_clo->environment);
    return whnf(out, make_clo_.make_clo(body, extended), reductions_left);
}

#endif
