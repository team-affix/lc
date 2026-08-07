#ifndef NBE_FIXTURE_HPP
#define NBE_FIXTURE_HPP

#include "debug_assert.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include <cstdint>
#include <limits>

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
        for(uint32_t i = 0; i < n; ++i)
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
    const expr* and_comb() { return lm(lm(ap(ap(dv(1), dv(0)), dv(1)))); }
    const expr* or_comb() { return lm(lm(ap(ap(dv(1), dv(1)), dv(0)))); }
    const expr* not_comb() {
        return lm(ap(ap(dv(0), false_comb()), true_comb()));
    }

    const expr* w_term() { return lm(lm(ap(ap(dv(1), dv(0)), dv(0)))); }

    const expr* b_term() {
        return lm(lm(lm(ap(dv(2), ap(dv(1), dv(0))))));
    }

    const expr* pred_comb() {
        const expr* step = lm(lm(ap(dv(0), ap(dv(1), dv(3)))));
        return lm(lm(lm(ap(ap(ap(dv(2), step), lm(dv(1))), lm(dv(0))))));
    }

    const expr* iszero_comb() {
        return lm(ap(ap(dv(0), lm(false_comb())), true_comb()));
    }

    const expr* if_comb() {
        return lm(lm(lm(ap(ap(dv(2), dv(1)), dv(0)))));
    }

    // Ω = (λx. x x)(λx. x x) — diverges under β; for budget / non-strict tests.
    const expr* omega_term() {
        const expr* xx = lm(ap(dv(0), dv(0)));
        return ap(xx, xx);
    }

    // Y = λf.(λx. f (x x)) (λx. f (x x))
    const expr* y_comb() {
        const expr* xx = lm(ap(dv(1), ap(dv(0), dv(0))));
        return lm(ap(xx, xx));
    }

    // exp = λm.λn. n m  (Church m^n)
    const expr* exp_comb() { return lm(lm(ap(dv(0), dv(1)))); }

    // fact = Y (λr. λn. if (iszero n) 1 (times n (r (pred n))))
    const expr* fact_comb() {
        const expr* else_branch =
            ap(ap(times_comb(), dv(0)), ap(dv(1), ap(pred_comb(), dv(0))));
        const expr* body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(0))), church(1)),
               else_branch);
        return ap(y_comb(), lm(lm(body)));
    }

    // Like fact_comb, but duplicates n into two distinct env cells so call-by-need
    // cannot share WHNF between the iszero/times use and the pred use:
    //   Y (λr. λn. (λn1. λn2. if (iszero n1) 1 (times n1 (r (pred n2)))) n n)
    const expr* fact_noshare_comb() {
        // under λr λn λn1 λn2: n2=0, n1=1, n=2, r=3
        const expr* else_branch =
            ap(ap(times_comb(), dv(1)), ap(dv(3), ap(pred_comb(), dv(0))));
        const expr* body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(1))), church(1)),
               else_branch);
        const expr* split = ap(ap(lm(lm(body)), dv(0)), dv(0));
        return ap(y_comb(), lm(lm(split)));
    }

    // fib = Y (λr. λn.
    //   if (iszero n) 0
    //   (if (iszero (pred n)) 1
    //    (plus (r (pred n)) (r (pred (pred n))))))
    const expr* fib_comb() {
        const expr* pred_n = ap(pred_comb(), dv(0));
        const expr* pred_pred_n = ap(pred_comb(), pred_n);
        const expr* else_branch =
            ap(ap(plus_comb(), ap(dv(1), pred_n)), ap(dv(1), pred_pred_n));
        const expr* inner_if =
            ap(ap(ap(if_comb(), ap(iszero_comb(), pred_n)), church(1)),
               else_branch);
        const expr* body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(0))), church(0)),
               inner_if);
        return ap(y_comb(), lm(lm(body)));
    }

    bool normalize_with_budget(const expr*& out, const expr* term,
                               uint64_t reductions_left) {
        env_pool envs;
        val_pool vals;
        env_lookup lookup;
        using red_t = reducer<val_pool, val_pool, env_pool, env_lookup>;
        using re_t =
            reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool,
                    val_pool, red_t>;
        red_t red{vals, vals, envs, lookup};
        re_t re{pool, pool, pool, vals, envs, vals, red};
        normalizer<val_pool, re_t> norm{vals, re};
        return norm.normalize(out, term, reductions_left);
    }

    const expr* normalize(const expr* term) {
        const expr* out;
        uint64_t budget = std::numeric_limits<uint64_t>::max();
        bool ok = normalize_with_budget(out, term, budget);
        DEBUG_ASSERT(ok);
        return out;
    }

    expr_pool pool;
};

#endif
