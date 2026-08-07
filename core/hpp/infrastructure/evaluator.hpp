#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <optional>
#include <variant>

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
struct evaluator {
    evaluator(IMakeClo& make_clo, IMakeNapp& make_napp,
              IMakeDelayed& make_delayed);
    std::optional<const val*> eval(const expr* term, const env* e,
                                   uint64_t& reductions_left);

private:
    std::optional<const val*> eval_app(const val* fun_val, const expr* arg,
                                       const env* arg_env,
                                       uint64_t& reductions_left);
    std::optional<const val*> lookup(uint32_t index, const env* e,
                                     uint64_t& reductions_left);

    IMakeClo& make_clo_;
    IMakeNapp& make_napp_;
    IMakeDelayed& make_delayed_;
};

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::evaluator(
    IMakeClo& make_clo, IMakeNapp& make_napp, IMakeDelayed& make_delayed)
    : make_clo_(make_clo), make_napp_(make_napp), make_delayed_(make_delayed) {}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
std::optional<const val*>
evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::eval(const expr* term,
                                                   const env* e,
                                                   uint64_t& reductions_left) {
    if (const expr::var* variable = std::get_if<expr::var>(&term->content)) {
        DEBUG_ASSERT(e != nullptr);
        return lookup(variable->index, e, reductions_left);
    }
    if (const expr::abs* abstraction = std::get_if<expr::abs>(&term->content))
        return make_clo_.make_clo(abstraction->body, e);
    const expr::app& application = std::get<expr::app>(term->content);
    std::optional<const val*> fun_val =
        eval(application.fun, e, reductions_left);
    if (!fun_val.has_value())
        return std::nullopt;
    return eval_app(*fun_val, application.arg, e, reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
std::optional<const val*> evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::eval_app(
    const val* fun_val, const expr* arg, const env* arg_env,
    uint64_t& reductions_left) {
    if (const val::clo* closure = std::get_if<val::clo>(&fun_val->content)) {
        if (reductions_left == 0)
            return std::nullopt;
        --reductions_left;
        const env* extended =
            make_delayed_.make_delayed(arg, arg_env, closure->captured);
        return eval(closure->body, extended, reductions_left);
    }
    return make_napp_.make_napp(fun_val, arg, arg_env);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
std::optional<const val*>
evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::lookup(uint32_t index,
                                                    const env* e,
                                                    uint64_t& reductions_left) {
    DEBUG_ASSERT(e != nullptr);
    if (index != 0) {
        DEBUG_ASSERT(e->parent != nullptr);
        return lookup(index - 1, e->parent, reductions_left);
    }
    if (const env::ready* bound = std::get_if<env::ready>(&e->binder))
        return bound->value;
    const env::delayed& thunk = std::get<env::delayed>(e->binder);
    std::optional<const val*> forced =
        eval(thunk.arg, thunk.arg_env, reductions_left);
    if (!forced.has_value())
        return std::nullopt;
    e->binder = env::ready{*forced};
    return forced;
}

#endif
