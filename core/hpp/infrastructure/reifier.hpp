#ifndef REIFIER_HPP
#define REIFIER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_clo_stage.hpp"
#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_napp_stage.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_stage.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <optional>
#include <variant>

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
struct reifier {
    reifier(IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
            IMakeFvar& make_fvar, IMakeEnv& make_env, IMakeClo& make_clo);
    std::optional<frame> process_val(reify_val_frame& f);
    std::optional<frame> process_clo(reify_clo_frame& f);
    std::optional<frame> process_napp(reify_napp_frame& f);

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
std::optional<frame>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process_val(
    reify_val_frame& f) {
    if(f.st == reify_val_stage::need_whnf) {
        f.st = reify_val_stage::after_whnf;
        return reduce_whnf_frame{f.v, reduce_whnf_stage::start};
    }
    if(f.st == reify_val_stage::after_whnf) {
        if(const val::clo* closure = std::get_if<val::clo>(&f.v->content)) {
            f.st = reify_val_stage::after_quote;
            return reify_clo_frame{f.out, closure, f.depth, nullptr, nullptr,
                                   reify_clo_stage::need_body};
        }
        if(const val::fvar* fresh = std::get_if<val::fvar>(&f.v->content)) {
            DEBUG_ASSERT(f.depth > fresh->depth);
            f.out = make_var_.make_var(f.depth - fresh->depth - 1);
            return std::nullopt;
        }
        f.st = reify_val_stage::after_quote;
        return reify_napp_frame{f.out, std::get<val::napp>(f.v->content),
                                f.depth, nullptr, nullptr, nullptr, nullptr,
                                reify_napp_stage::need_head};
    }
    DEBUG_ASSERT(f.st == reify_val_stage::after_quote);
    return std::nullopt;
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::optional<frame>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv, IMakeClo>::process_clo(
    reify_clo_frame& f) {
    if(f.st == reify_clo_stage::need_body) {
        DEBUG_ASSERT(
            std::holds_alternative<expr::abs>(f.closure->term->content));
        const val* fresh = make_fvar_.make_fvar(f.depth);
        env* extended = make_env_.make_env(fresh, f.closure->environment);
        const expr* body = std::get<expr::abs>(f.closure->term->content).body;
        f.body_holder = make_clo_.make_clo(body, extended);
        f.st = reify_clo_stage::after_body;
        return reify_val_frame{f.body_nf, f.body_holder, f.depth + 1,
                               reify_val_stage::need_whnf};
    }
    DEBUG_ASSERT(f.st == reify_clo_stage::after_body);
    f.out = make_abs_.make_abs(f.body_nf);
    return std::nullopt;
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeEnv, typename IMakeClo>
std::optional<frame> reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeEnv,
                             IMakeClo>::process_napp(reify_napp_frame& f) {
    if(f.st == reify_napp_stage::need_head) {
        f.head_holder = f.neutral.head;
        f.st = reify_napp_stage::need_arg;
        return reify_val_frame{f.fun_term, f.head_holder, f.depth,
                               reify_val_stage::need_whnf};
    }
    if(f.st == reify_napp_stage::need_arg) {
        f.arg_holder = make_clo_.make_clo(f.neutral.arg, f.neutral.arg_env);
        f.st = reify_napp_stage::after_arg;
        return reify_val_frame{f.arg_term, f.arg_holder, f.depth,
                               reify_val_stage::need_whnf};
    }
    DEBUG_ASSERT(f.st == reify_napp_stage::after_arg);
    f.out = make_app_.make_app(f.fun_term, f.arg_term);
    return std::nullopt;
}

#endif
