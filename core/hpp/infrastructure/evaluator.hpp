#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "debug_assert.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <variant>

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
struct evaluator {
    evaluator(IMakeClo& make_clo, IMakeNapp& make_napp,
              IMakeDelayed& make_delayed);
    bool eval(const val*& out, const expr* term, env* e,
              uint64_t& reductions_left);

private:
    bool resolve(const val*& out, uint32_t index, env* e,
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
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::eval(
    const val*& out, const expr* term, env* e, uint64_t& reductions_left) {
    if (const expr::var* variable = std::get_if<expr::var>(&term->content)) {
        DEBUG_ASSERT(e != nullptr);
        return resolve(out, variable->index, e, reductions_left);
    }
    if (const expr::abs* abstraction = std::get_if<expr::abs>(&term->content)) {
        out = make_clo_.make_clo(abstraction->body, e);
        return true;
    }
    const expr::app& application = std::get<expr::app>(term->content);
    const val* fun_val;
    if (!eval(fun_val, application.fun, e, reductions_left))
        return false;
    const val::clo* closure = std::get_if<val::clo>(&fun_val->content);
    if (closure == nullptr) {
        out = make_napp_.make_napp(fun_val, application.arg, e);
        return true;
    }
    if (reductions_left == 0)
        return false;
    --reductions_left;
    env* extended =
        make_delayed_.make_delayed(application.arg, e, closure->captured);
    return eval(out, closure->body, extended, reductions_left);
}

template <typename IMakeClo, typename IMakeNapp, typename IMakeDelayed>
bool evaluator<IMakeClo, IMakeNapp, IMakeDelayed>::resolve(
    const val*& out, uint32_t index, env* e, uint64_t& reductions_left) {
    DEBUG_ASSERT(e != nullptr);
    if (index != 0) {
        DEBUG_ASSERT(e->parent != nullptr);
        return resolve(out, index - 1, e->parent, reductions_left);
    }
    if (const env::ready* bound = std::get_if<env::ready>(&e->binder)) {
        out = bound->value;
        return true;
    }
    const env::delayed& thunk = std::get<env::delayed>(e->binder);
    if (!eval(out, thunk.arg, thunk.arg_env, reductions_left))
        return false;
    e->binder = env::ready{out};
    return true;
}

#endif
