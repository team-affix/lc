#ifndef REDUCER_HPP
#define REDUCER_HPP

#include <optional>
#include <utility>
#include <variant>
#include "value_objects/app_after_body_stage.hpp"
#include "value_objects/app_after_fun_stage.hpp"
#include "value_objects/app_need_fun_stage.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_app_stage.hpp"
#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_var_stage.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/val.hpp"
#include "value_objects/var_after_force_stage.hpp"
#include "value_objects/var_start_stage.hpp"
#include "value_objects/whnf_after_child_stage.hpp"
#include "value_objects/whnf_start_stage.hpp"
#include "debug_assert.hpp"

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
struct reducer {
    reducer(IMakeClo& make_clo, IMakeNapp& make_napp, IMakeEnv& make_env,
            ILookup& lookup);
    std::optional<std::pair<reduce_whnf_stage, funcall>>
    process(reduce_whnf_frame& f, whnf_start_stage);
    std::optional<std::pair<reduce_whnf_stage, funcall>>
    process(reduce_whnf_frame& f, whnf_after_child_stage);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_need_fun_stage);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_after_fun_stage);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_after_body_stage);
    std::optional<std::pair<reduce_var_stage, funcall>>
    process(reduce_var_frame& f, var_start_stage);
    std::optional<std::pair<reduce_var_stage, funcall>>
    process(reduce_var_frame& f, var_after_force_stage);

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
std::optional<std::pair<reduce_whnf_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(reduce_whnf_frame& f,
                                                         whnf_start_stage) {
    if(!std::holds_alternative<val::clo>(f.slot->content))
        return std::nullopt;

    auto& closure = std::get<val::clo>(f.slot->content);

    const expr* term = closure.term;
    if(std::holds_alternative<expr::abs>(term->content))
        return std::nullopt;
    if(const expr::var* variable = std::get_if<expr::var>(&term->content)) {
        return std::pair<reduce_whnf_stage, funcall>{
            whnf_after_child_stage{},
            reduce_var_funcall{f.slot, variable, closure.environment}};
    }
    return std::pair<reduce_whnf_stage, funcall>{
        whnf_after_child_stage{},
        reduce_app_funcall{f.slot, &std::get<expr::app>(term->content),
                           closure.environment}};
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_whnf_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(
    reduce_whnf_frame&, whnf_after_child_stage) {
    return std::nullopt;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_app_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(reduce_app_frame& f,
                                                         app_need_fun_stage) {
    f.fun_holder = make_clo_.make_clo(f.application->fun, f.e);
    return std::pair<reduce_app_stage, funcall>{
        app_after_fun_stage{}, reduce_whnf_funcall{f.fun_holder}};
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_app_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(reduce_app_frame& f,
                                                         app_after_fun_stage) {
    if(!std::holds_alternative<val::clo>(f.fun_holder->content)) {
        f.slot = make_napp_.make_napp(f.fun_holder, f.application->arg, f.e);
        return std::nullopt;
    }
    auto& fun_clo = std::get<val::clo>(f.fun_holder->content);
    DEBUG_ASSERT(std::holds_alternative<expr::abs>(fun_clo.term->content));
    const expr* body = std::get<expr::abs>(fun_clo.term->content).body;
    env* extended = make_env_.make_env(
        make_clo_.make_clo(f.application->arg, f.e), fun_clo.environment);
    f.slot = make_clo_.make_clo(body, extended);
    return std::pair<reduce_app_stage, funcall>{app_after_body_stage{},
                                                reduce_whnf_funcall{f.slot}};
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_app_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(reduce_app_frame&,
                                                         app_after_body_stage) {
    return std::nullopt;
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_var_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(reduce_var_frame& f,
                                                         var_start_stage) {
    f.cell = lookup_.lookup(f.e, f.variable->index);
    return std::pair<reduce_var_stage, funcall>{
        var_after_force_stage{}, reduce_whnf_funcall{f.cell->bound_value}};
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeEnv,
          typename ILookup>
std::optional<std::pair<reduce_var_stage, funcall>>
reducer<IMakeClo, IMakeNapp, IMakeEnv, ILookup>::process(
    reduce_var_frame& f, var_after_force_stage) {
    f.slot = f.cell->bound_value;
    return std::nullopt;
}

#endif
