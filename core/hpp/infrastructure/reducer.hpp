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
    bool whnf(const val*& v, uint64_t& reductions_left);

  private:
    bool whnf_clo(const val*& v, const val::clo* closure,
                  uint64_t& reductions_left);
    bool whnf_fvar(const val*& v, const val::fvar* fresh,
                   uint64_t& reductions_left);
    bool whnf_napp(const val*& v, const val::napp* neutral,
                   uint64_t& reductions_left);

    bool whnf_var(const val*& v, const expr::var* variable, env* e,
                  uint64_t& reductions_left);
    bool whnf_abs(const val*& v, const expr::abs* abstraction,
                  uint64_t& reductions_left);
    bool whnf_app(const val*& v, const expr::app* application, env* e,
                  uint64_t& reductions_left);

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
    const val*& v, uint64_t& reductions_left) {
    if(const val::clo* closure = std::get_if<val::clo>(&v->content))
        return whnf_clo(v, closure, reductions_left);
    if(const val::fvar* fresh = std::get_if<val::fvar>(&v->content))
        return whnf_fvar(v, fresh, reductions_left);
    return whnf_napp(v, &std::get<val::napp>(v->content), reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_clo(
    const val*& v, const val::clo* closure, uint64_t& reductions_left) {
    const expr* term = closure->term;
    if(const expr::var* variable = std::get_if<expr::var>(&term->content))
        return whnf_var(v, variable, closure->environment, reductions_left);
    if(const expr::abs* abstraction = std::get_if<expr::abs>(&term->content))
        return whnf_abs(v, abstraction, reductions_left);
    return whnf_app(v, &std::get<expr::app>(term->content),
                    closure->environment, reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_fvar(
    const val*&, const val::fvar*, uint64_t&) {
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_napp(
    const val*&, const val::napp*, uint64_t&) {
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_var(
    const val*& v, const expr::var* variable, env* e,
    uint64_t& reductions_left) {
    ///////////
    //// LOOKUP
    ///////////
    env* cell = lookup_.lookup(e, variable->index);

    ///////////
    //// WHNF + OVERWRITE
    ///////////
    if(!whnf(cell->bound_value, reductions_left))
        return false;
    v = cell->bound_value;
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_abs(
    const val*&, const expr::abs*, uint64_t&) {
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeFrame,
          typename ILookup>
bool reducer<IMakeClo, IMakeNapp, IMakeFrame, ILookup>::whnf_app(
    const val*& v, const expr::app* application, env* e,
    uint64_t& reductions_left) {
    ///////////
    //// WHNF FUN
    ///////////
    const val* fun_val = make_clo_.make_clo(application->fun, e);
    if(!whnf(fun_val, reductions_left))
        return false;
    const val::clo* fun_clo = std::get_if<val::clo>(&fun_val->content);

    ///////////
    //// APPLY NEUTRAL
    ///////////
    if(fun_clo == nullptr) {
        v = make_napp_.make_napp(fun_val, application->arg, e);
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
        make_clo_.make_clo(application->arg, e), fun_clo->environment);
    v = make_clo_.make_clo(body, extended);
    return whnf(v, reductions_left);
}

#endif
