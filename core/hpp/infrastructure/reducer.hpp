#ifndef REDUCER_HPP
#define REDUCER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_app_stage.hpp"
#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_var_stage.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/val.hpp"
#include <optional>
#include <variant>

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
struct reducer {
    reducer(IMakeClo& make_clo, IMakeNapp& make_napp, IMakeEnv& make_env,
            ILookup& lookup);
    std::optional<frame> process_whnf(reduce_whnf_frame& f);
    std::optional<frame> process_app(reduce_app_frame& f);
    std::optional<frame> process_var(reduce_var_frame& f);

  private:
    IMakeClo& make_clo_;
    IMakeNapp& make_napp_;
    IMakeEnv& make_env_;
    ILookup& lookup_;
};

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::reducer(IMakeClo& make_clo,
                                                         IMakeNapp& make_napp,
                                                         IMakeEnv& make_env,
                                                         ILookup& lookup)
    : make_clo_(make_clo), make_napp_(make_napp), make_env_(make_env),
      lookup_(lookup) {
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<frame>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process_whnf(
    reduce_whnf_frame& f) {
    if(f.st == reduce_whnf_stage::after_child)
        return std::nullopt;

    DEBUG_ASSERT(f.st == reduce_whnf_stage::start);

    if(const val::clo* closure = std::get_if<val::clo>(&f.slot->content)) {
        const expr* term = closure->term;
        if(std::get_if<expr::abs>(&term->content) != nullptr)
            return std::nullopt;
        if(const expr::var* variable = std::get_if<expr::var>(&term->content)) {
            f.st = reduce_whnf_stage::after_child;
            return reduce_var_frame{f.slot, variable, closure->environment,
                                    nullptr, reduce_var_stage::start};
        }
        f.st = reduce_whnf_stage::after_child;
        return reduce_app_frame{f.slot, &std::get<expr::app>(term->content),
                                closure->environment, nullptr,
                                reduce_app_stage::need_fun};
    }
    if(std::get_if<val::fvar>(&f.slot->content) != nullptr)
        return std::nullopt;
    DEBUG_ASSERT(std::holds_alternative<val::napp>(f.slot->content));
    return std::nullopt;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<frame>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process_var(
    reduce_var_frame& f) {
    if(f.st == reduce_var_stage::start) {
        f.cell = lookup_.lookup(f.e, f.variable->index);
        f.st = reduce_var_stage::after_force;
        return reduce_whnf_frame{f.cell->bound_value, reduce_whnf_stage::start};
    }
    DEBUG_ASSERT(f.st == reduce_var_stage::after_force);
    f.slot = f.cell->bound_value;
    return std::nullopt;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<frame>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process_app(
    reduce_app_frame& f) {
    if(f.st == reduce_app_stage::need_fun) {
        f.fun_holder = make_clo_.make_clo(f.application->fun, f.e);
        f.st = reduce_app_stage::after_fun;
        return reduce_whnf_frame{f.fun_holder, reduce_whnf_stage::start};
    }
    if(f.st == reduce_app_stage::after_fun) {
        const val::clo* fun_clo = std::get_if<val::clo>(&f.fun_holder->content);
        if(fun_clo == nullptr) {
            f.slot =
                make_napp_.make_napp(f.fun_holder, f.application->arg, f.e);
            return std::nullopt;
        }
        DEBUG_ASSERT(std::holds_alternative<expr::abs>(fun_clo->term->content));
        const expr* body = std::get<expr::abs>(fun_clo->term->content).body;
        env* extended = make_env_.make_env(
            make_clo_.make_clo(f.application->arg, f.e), fun_clo->environment);
        f.slot = make_clo_.make_clo(body, extended);
        f.st = reduce_app_stage::after_body;
        return reduce_whnf_frame{f.slot, reduce_whnf_stage::start};
    }
    DEBUG_ASSERT(f.st == reduce_app_stage::after_body);
    return std::nullopt;
}

#endif
