#ifndef NBE_FIXTURE_HPP
#define NBE_FIXTURE_HPP

#include "infrastructure/env_pool.hpp"
#include "infrastructure/evaluator.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"

// Shared term builders + fresh env/val pools per normalize.
struct nbe_fixture {
    nbe_fixture() : pool() {}

    const expr* dv(uint32_t index) { return pool.make_var(index); }
    const expr* lm(const expr* body) { return pool.make_abs(body); }
    const expr* ap(const expr* fun, const expr* arg) {
        return pool.make_app(fun, arg);
    }

    const expr* id_term() { return lm(dv(0)); }
    const expr* k_term() { return lm(lm(dv(1))); }
    const expr* s_term() {
        return lm(lm(lm(ap(ap(dv(2), dv(0)), ap(dv(1), dv(0))))));
    }

    const expr* church(uint32_t n) {
        const expr* body = dv(0);
        for (uint32_t i = 0; i < n; ++i)
            body = ap(dv(1), body);
        return lm(lm(body));
    }

    const expr* succ_comb() {
        return lm(lm(lm(ap(dv(1), ap(ap(dv(2), dv(1)), dv(0))))));
    }

    const expr* plus_comb() {
        return lm(lm(lm(lm(ap(ap(dv(3), dv(1)), ap(ap(dv(2), dv(1)), dv(0)))))));
    }

    const expr* times_comb() {
        return lm(lm(lm(ap(dv(2), ap(dv(1), dv(0))))));
    }

    const expr* true_comb() { return lm(lm(dv(1))); }
    const expr* false_comb() { return lm(lm(dv(0))); }
    const expr* and_comb() {
        return lm(lm(ap(ap(dv(1), dv(0)), dv(1))));
    }
    const expr* or_comb() {
        return lm(lm(ap(ap(dv(1), dv(1)), dv(0))));
    }
    const expr* not_comb() {
        return lm(ap(ap(dv(0), false_comb()), true_comb()));
    }

    const expr* normalize(const expr* term) {
        env_pool envs;
        val_pool vals;
        evaluator<val_pool, val_pool, env_pool> ev{vals, vals, envs};
        reifier<expr_pool,
                expr_pool,
                expr_pool,
                val_pool,
                env_pool,
                evaluator<val_pool, val_pool, env_pool>>
            re{pool, pool, pool, vals, envs, ev};
        normalizer<evaluator<val_pool, val_pool, env_pool>,
                   reifier<expr_pool,
                           expr_pool,
                           expr_pool,
                           val_pool,
                           env_pool,
                           evaluator<val_pool, val_pool, env_pool>>>
            norm{ev, re};
        return norm.normalize(term);
    }

    expr_pool pool;
};

#endif
