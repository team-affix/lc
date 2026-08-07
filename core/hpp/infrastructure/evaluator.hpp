#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <variant>

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
struct evaluator {
    evaluator(IMakeClo& make_clo, IMakeNapp& make_napp,
              IMakeDelayed& make_delayed, ILookup& lookup);
    bool eval(const val*& out, const expr* term, env* e,
              uint64_t& reductions_left);

  private:
    bool eval_var(const val*& out, const expr::var* variable, env* e,
                  uint64_t& reductions_left);
    bool eval_abs(const val*& out, const expr::abs* abstraction, env* e,
                  uint64_t& reductions_left);
    bool eval_app(const val*& out, const expr::app& application, env* e,
                  uint64_t& reductions_left);

    IMakeClo& make_clo_;
    IMakeNapp& make_napp_;
    IMakeDelayed& make_delayed_;
    ILookup& lookup_;
};

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
evaluator<IMakeClo, IMakeNapp, IMakeDelayed, ILookup>::evaluator(
    IMakeClo& make_clo, IMakeNapp& make_napp, IMakeDelayed& make_delayed,
    ILookup& lookup)
    : make_clo_(make_clo), make_napp_(make_napp), make_delayed_(make_delayed),
      lookup_(lookup) {
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed, ILookup>::eval(
    const val*& out, const expr* term, env* e, uint64_t& reductions_left) {
    if(const expr::var* variable = std::get_if<expr::var>(&term->content))
        return eval_var(out, variable, e, reductions_left);
    if(const expr::abs* abstraction = std::get_if<expr::abs>(&term->content))
        return eval_abs(out, abstraction, e, reductions_left);
    return eval_app(out, std::get<expr::app>(term->content), e,
                    reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed, ILookup>::eval_var(
    const val*& out, const expr::var* variable, env* e,
    uint64_t& reductions_left) {
    ///////////
    //// LOOKUP
    ///////////
    env* cell = lookup_.lookup(e, variable->index);

    ///////////
    //// READY
    ///////////
    if(const env::ready* bound = std::get_if<env::ready>(&cell->binder)) {
        out = bound->value;
        return true;
    }

    ///////////
    //// FORCE DELAYED
    ///////////
    const env::delayed& thunk = std::get<env::delayed>(cell->binder);
    if(!eval(out, thunk.arg, thunk.arg_env, reductions_left))
        return false;
    cell->binder = env::ready{out};
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed, ILookup>::eval_abs(
    const val*& out, const expr::abs* abstraction, env* e, uint64_t&) {
    out = make_clo_.make_clo(abstraction->body, e);
    return true;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed,
          typename ILookup>
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed, ILookup>::eval_app(
    const val*& out, const expr::app& application, env* e,
    uint64_t& reductions_left) {
    ///////////
    //// EVAL FUN
    ///////////
    const val* fun_val;
    if(!eval(fun_val, application.fun, e, reductions_left))
        return false;
    const val::clo* closure = std::get_if<val::clo>(&fun_val->content);

    ///////////
    //// APPLY NEUTRAL
    ///////////
    if(closure == nullptr) {
        out = make_napp_.make_napp(fun_val, application.arg, e);
        return true;
    }

    ///////////
    //// APPLY CLO (BETA)
    ///////////
    if(reductions_left == 0)
        return false;
    --reductions_left;
    env* extended =
        make_delayed_.make_delayed(application.arg, e, closure->captured);
    return eval(out, closure->body, extended, reductions_left);
}

#endif
