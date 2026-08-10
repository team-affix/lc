#ifndef REIFIER_HPP
#define REIFIER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_clo_after_body_stage.hpp"
#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_clo_need_body_stage.hpp"
#include "value_objects/reify_clo_stage.hpp"
#include "value_objects/reify_napp_after_arg_stage.hpp"
#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_napp_need_arg_stage.hpp"
#include "value_objects/reify_napp_need_head_stage.hpp"
#include "value_objects/reify_napp_stage.hpp"
#include "value_objects/reify_val_after_quote_stage.hpp"
#include "value_objects/reify_val_after_whnf_stage.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_need_whnf_stage.hpp"
#include "value_objects/reify_val_stage.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <optional>
#include <utility>
#include <variant>

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
struct reifier {
    reifier(IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
            IMakeFvar& make_fvar, IMakeEnv& make_env, IMakeClo& make_clo);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_need_whnf_stage);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_after_whnf_stage);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_after_quote_stage);
    std::pair<reify_clo_stage, std::optional<funcall>>
    process(reify_clo_frame& f, reify_clo_need_body_stage);
    std::pair<reify_clo_stage, std::optional<funcall>>
    process(reify_clo_frame& f, reify_clo_after_body_stage);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_need_head_stage);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_need_arg_stage);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_after_arg_stage);

  private:
    IMakeVar& make_var_;
    IMakeAbs& make_abs_;
    IMakeApp& make_app_;
    IMakeFvar& make_fvar_;
    IMakeEnv& make_env_;
    IMakeClo& make_clo_;
};

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::reifier(
    IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
    IMakeFvar& make_fvar, IMakeEnv& make_env, IMakeClo& make_clo)
    : make_var_(make_var), make_abs_(make_abs), make_app_(make_app),
      make_fvar_(make_fvar), make_env_(make_env), make_clo_(make_clo) {
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_val_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_val_frame& f, reify_val_need_whnf_stage) {
    return {reify_val_after_whnf_stage{}, reduce_whnf_funcall{f.v}};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_val_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_val_frame& f, reify_val_after_whnf_stage) {
    if(const val::clo* closure = std::get_if<val::clo>(&f.v->content)) {
        return {reify_val_after_quote_stage{},
                reify_clo_funcall{f.out, closure, f.depth}};
    }
    if(const val::fvar* fresh = std::get_if<val::fvar>(&f.v->content)) {
        DEBUG_ASSERT(f.depth > fresh->depth);
        f.out = make_var_.make_var(f.depth - fresh->depth - 1);
        return {reify_val_after_whnf_stage{}, std::nullopt};
    }
    return {reify_val_after_quote_stage{},
            reify_napp_funcall{f.out, std::get<val::napp>(f.v->content),
                               f.depth}};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_val_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_val_frame&, reify_val_after_quote_stage) {
    return {reify_val_after_quote_stage{}, std::nullopt};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_clo_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_clo_frame& f, reify_clo_need_body_stage) {
    DEBUG_ASSERT(std::holds_alternative<expr::abs>(f.closure->term->content));
    const val* fresh = make_fvar_.make_fvar(f.depth);
    env* extended = make_env_.make_env(fresh, f.closure->environment);
    const expr* body = std::get<expr::abs>(f.closure->term->content).body;
    f.body_holder = make_clo_.make_clo(body, extended);
    return {reify_clo_after_body_stage{},
            reify_val_funcall{f.body_nf, f.body_holder, f.depth + 1}};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_clo_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_clo_frame& f, reify_clo_after_body_stage) {
    f.out = make_abs_.make_abs(f.body_nf);
    return {reify_clo_after_body_stage{}, std::nullopt};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_napp_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_napp_frame& f, reify_napp_need_head_stage) {
    f.head_holder = f.neutral.head;
    return {reify_napp_need_arg_stage{},
            reify_val_funcall{f.fun_term, f.head_holder, f.depth}};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_napp_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_napp_frame& f, reify_napp_need_arg_stage) {
    f.arg_holder = make_clo_.make_clo(f.neutral.arg, f.neutral.arg_env);
    return {reify_napp_after_arg_stage{},
            reify_val_funcall{f.arg_term, f.arg_holder, f.depth}};
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::pair<reify_napp_stage, std::optional<funcall>>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process(
    reify_napp_frame& f, reify_napp_after_arg_stage) {
    f.out = make_app_.make_app(f.fun_term, f.arg_term);
    return {reify_napp_after_arg_stage{}, std::nullopt};
}

#endif
