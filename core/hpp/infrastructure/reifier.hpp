#ifndef REIFIER_HPP
#define REIFIER_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <variant>

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
struct reifier {
    reifier(IMakeVar& make_var, IMakeAbs& make_abs, IMakeApp& make_app,
            IMakeFvar& make_fvar, IMakeFrame& make_frame, IMakeClo& make_clo,
            IWhnf& whnf);
    bool reify(const expr*& out, const val*& v, uint32_t depth,
               uint64_t& reductions_left);

  private:
    bool reify_clo(const expr*& out, const val::clo* closure, uint32_t depth,
                   uint64_t& reductions_left);
    bool reify_fvar(const expr*& out, const val::fvar* fresh, uint32_t depth,
                    uint64_t& reductions_left);
    bool reify_napp(const expr*& out, const val::napp& neutral, uint32_t depth,
                    uint64_t& reductions_left);

    IMakeVar& make_var_;
    IMakeAbs& make_abs_;
    IMakeApp& make_app_;
    IMakeFvar& make_fvar_;
    IMakeFrame& make_frame_;
    IMakeClo& make_clo_;
    IWhnf& whnf_;
};

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeFrame, IMakeClo,
        IWhnf>::reifier(IMakeVar& make_var, IMakeAbs& make_abs,
                        IMakeApp& make_app, IMakeFvar& make_fvar,
                        IMakeFrame& make_frame, IMakeClo& make_clo, IWhnf& whnf)
    : make_var_(make_var), make_abs_(make_abs), make_app_(make_app),
      make_fvar_(make_fvar), make_frame_(make_frame), make_clo_(make_clo),
      whnf_(whnf) {
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
bool reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeFrame, IMakeClo,
             IWhnf>::reify(const expr*& out, const val*& v, uint32_t depth,
                           uint64_t& reductions_left) {
    ///////////
    //// WHNF
    ///////////
    if(!whnf_.whnf(v, reductions_left))
        return false;

    if(const val::clo* closure = std::get_if<val::clo>(&v->content))
        return reify_clo(out, closure, depth, reductions_left);
    if(const val::fvar* fresh = std::get_if<val::fvar>(&v->content))
        return reify_fvar(out, fresh, depth, reductions_left);
    return reify_napp(out, std::get<val::napp>(v->content), depth,
                      reductions_left);
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
bool reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeFrame, IMakeClo,
             IWhnf>::reify_clo(const expr*& out, const val::clo* closure,
                               uint32_t depth, uint64_t& reductions_left) {
    DEBUG_ASSERT(std::holds_alternative<expr::abs>(closure->term->content));

    ///////////
    //// EXTEND WITH FVAR
    ///////////
    const val* fresh = make_fvar_.make_fvar(depth);
    env* extended = make_frame_.make_frame(fresh, closure->environment);

    ///////////
    //// REIFY BODY
    ///////////
    const expr* body = std::get<expr::abs>(closure->term->content).body;
    const val* body_val = make_clo_.make_clo(body, extended);
    const expr* body_nf;
    if(!reify(body_nf, body_val, depth + 1, reductions_left))
        return false;
    out = make_abs_.make_abs(body_nf);
    return true;
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
bool reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeFrame, IMakeClo,
             IWhnf>::reify_fvar(const expr*& out, const val::fvar* fresh,
                                uint32_t depth, uint64_t&) {
    DEBUG_ASSERT(depth > fresh->depth);
    out = make_var_.make_var(depth - fresh->depth - 1);
    return true;
}

template <typename IMakeVar, typename IMakeAbs, typename IMakeApp,
          typename IMakeFvar, typename IMakeFrame, typename IMakeClo,
          typename IWhnf>
bool reifier<IMakeVar, IMakeAbs, IMakeApp, IMakeFvar, IMakeFrame, IMakeClo,
             IWhnf>::reify_napp(const expr*& out, const val::napp& neutral,
                                uint32_t depth, uint64_t& reductions_left) {
    ///////////
    //// REIFY HEAD
    ///////////
    const val* head = neutral.head;
    const expr* fun_term;
    if(!reify(fun_term, head, depth, reductions_left))
        return false;

    ///////////
    //// REIFY ARG
    ///////////
    const val* arg_val = make_clo_.make_clo(neutral.arg, neutral.arg_env);
    const expr* arg_term;
    if(!reify(arg_term, arg_val, depth, reductions_left))
        return false;
    out = make_app_.make_app(fun_term, arg_term);
    return true;
}

#endif
