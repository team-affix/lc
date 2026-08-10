#ifndef NBE_FIXTURE_HPP
#define NBE_FIXTURE_HPP

#include "debug_assert.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/continuation.hpp"
#include <cstdint>

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

    // Ω = (λx. x x)(λx. x x) — diverges under β; for non-strict / early-stop tests.
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

    // pair = λa.λb.λf. f a b;  fst / snd project.
    const expr* pair_comb() {
        return lm(lm(lm(ap(ap(dv(0), dv(2)), dv(1)))));
    }
    const expr* fst_comb() { return lm(ap(dv(0), lm(lm(dv(1))))); }
    const expr* snd_comb() { return lm(ap(dv(0), lm(lm(dv(0))))); }

    // fact = Y (λr. λn. if (iszero n) 1 (times n (r (pred n))))
    const expr* fact_comb() {
        const expr* else_branch =
            ap(ap(times_comb(), dv(0)), ap(dv(1), ap(pred_comb(), dv(0))));
        const expr* body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(0))), church(1)),
               else_branch);
        return ap(y_comb(), lm(lm(body)));
    }

    // Iterative fact via Church n as a loop (no Y):
    //   fact n = snd (n (λp. (λk. pair k (× k (snd p))) (succ (fst p)))
    //                  (pair 0 1))
    const expr* fact_iter_comb() {
        // under λp λk: k=0, p=1
        const expr* with_k =
            ap(ap(pair_comb(), dv(0)),
               ap(ap(times_comb(), dv(0)), ap(snd_comb(), dv(1))));
        const expr* step =
            lm(ap(lm(with_k), ap(succ_comb(), ap(fst_comb(), dv(0)))));
        const expr* init =
            ap(ap(pair_comb(), church(0)), church(1));
        // under λn: n=0
        return lm(ap(snd_comb(), ap(ap(dv(0), step), init)));
    }

    // Little-endian binary: Church list of bits (true=1, false=0), LSB at head.
    // nil = λc.λn. n;  cons = λh.λt.λc.λn. c h t
    const expr* bin_nil() { return lm(lm(dv(0))); }
    const expr* bin_cons() {
        return lm(lm(lm(lm(ap(ap(dv(1), dv(3)), dv(2))))));
    }
    const expr* bin(uint32_t n) {
        if(n == 0)
            return bin_nil();
        const expr* bit = (n & 1u) != 0u ? true_comb() : false_comb();
        return ap(ap(bin_cons(), bit), bin(n >> 1));
    }

    // succ [] = [1];  succ (0:xs) = 1:xs;  succ (1:xs) = 0:(succ xs)
    const expr* bin_succ_comb() {
        // under λs λxs λb λbs: bs=0, b=1, xs=2, s=3
        const expr* on_cons = lm(lm(ap(
            ap(dv(1),
               ap(ap(bin_cons(), false_comb()), ap(dv(3), dv(0)))),
            ap(ap(bin_cons(), true_comb()), dv(0)))));
        const expr* body = lm(ap(ap(dv(0), on_cons),
                                 ap(ap(bin_cons(), true_comb()), bin_nil())));
        return ap(y_comb(), lm(body));
    }

    // dbl [] = [];  dbl xs = 0:xs  (×2)
    const expr* bin_dbl_comb() {
        return lm(ap(ap(dv(0), lm(lm(ap(ap(bin_cons(), false_comb()), dv(2))))),
                     bin_nil()));
    }

    const expr* bin_xor_comb() {
        // λa.λb. a (not b) b
        return lm(lm(ap(ap(dv(1), ap(not_comb(), dv(0))), dv(0))));
    }
    const expr* bin_maj_comb() {
        // λx.λy.λc. (x∧y) ∨ (x∧c) ∨ (y∧c)
        const expr* xy = ap(ap(and_comb(), dv(2)), dv(1));
        const expr* xc = ap(ap(and_comb(), dv(2)), dv(0));
        const expr* yc = ap(ap(and_comb(), dv(1)), dv(0));
        return lm(lm(lm(ap(ap(or_comb(), xy), ap(ap(or_comb(), xc), yc)))));
    }

    // addc xs ys cin — schoolbook add, bits LE
    const expr* bin_addc_comb() {
        // under λr λxs λys λcin λx λxs' λy λys':
        //   ys'=0 y=1 xs'=2 x=3 cin=4 ys=5 xs=6 r=7
        const expr* sum_bit = ap(
            ap(bin_xor_comb(), ap(ap(bin_xor_comb(), dv(3)), dv(1))), dv(4));
        const expr* cout =
            ap(ap(ap(bin_maj_comb(), dv(3)), dv(1)), dv(4));
        const expr* both = ap(
            ap(bin_cons(), sum_bit),
            ap(ap(ap(dv(7), dv(2)), dv(0)), cout));
        const expr* on_y = lm(lm(both));
        // under λr λxs λys λcin λx λxs': xs'=0 x=1 cin=2 ys=3 xs=4 r=5
        const expr* ys_nil = ap(
            ap(dv(2),
               ap(ap(dv(1),
                     ap(ap(bin_cons(), false_comb()),
                        ap(bin_succ_comb(), dv(0)))),
                  ap(ap(bin_cons(), true_comb()), dv(0)))),
            ap(ap(bin_cons(), dv(1)), dv(0)));
        const expr* on_x = lm(lm(ap(ap(dv(3), on_y), ys_nil)));
        // under λr λxs λys λcin: cin=0 ys=1 xs=2 r=3
        const expr* xs_nil_ys_nil = ap(
            ap(dv(0), ap(ap(bin_cons(), true_comb()), bin_nil())), bin_nil());
        // under λr λxs λys λcin λy λys': ys'=0 y=1 cin=2 ys=3 xs=4 r=5
        const expr* xs_nil_on_y = lm(lm(ap(
            ap(dv(2),
               ap(ap(dv(1),
                     ap(ap(bin_cons(), false_comb()),
                        ap(bin_succ_comb(), dv(0)))),
                  ap(ap(bin_cons(), true_comb()), dv(0)))),
            ap(ap(bin_cons(), dv(1)), dv(0)))));
        const expr* xs_nil =
            ap(ap(dv(1), xs_nil_on_y), xs_nil_ys_nil);
        // λxs λys λcin. xs on_x xs_nil
        const expr* body =
            lm(lm(lm(ap(ap(dv(2), on_x), xs_nil))));
        return ap(y_comb(), lm(body));
    }

    const expr* bin_add_comb() {
        return lm(lm(ap(ap(ap(bin_addc_comb(), dv(1)), dv(0)), false_comb())));
    }

    // mul xs ys: shift-and-add on LE bits
    const expr* bin_times_comb() {
        // under λm λxs λys λb λbs: bs=0 b=1 ys=2 xs=3 m=4
        const expr* shifted =
            ap(bin_dbl_comb(), ap(ap(dv(4), dv(0)), dv(2)));
        // under λm λxs λys λb λbs λp: p=0 bs=1 b=2 ys=3 xs=4 m=5
        const expr* with_p = ap(
            ap(dv(2), ap(ap(bin_add_comb(), dv(3)), dv(0))), dv(0));
        const expr* on_cons = lm(lm(ap(lm(with_p), shifted)));
        const expr* body =
            lm(lm(ap(ap(dv(1), on_cons), bin_nil()))); // λxs λys
        return ap(y_comb(), lm(body));
    }

    // Same iterative fact loop as fact_iter_comb, but counters/products are LE binary.
    const expr* fact_bin_iter_comb() {
        // under λp λk: k=0, p=1
        const expr* with_k =
            ap(ap(pair_comb(), dv(0)),
               ap(ap(bin_times_comb(), dv(0)), ap(snd_comb(), dv(1))));
        const expr* step = lm(
            ap(lm(with_k), ap(bin_succ_comb(), ap(fst_comb(), dv(0)))));
        const expr* init = ap(ap(pair_comb(), bin(0)), bin(1));
        return lm(ap(snd_comb(), ap(ap(dv(0), step), init)));
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

    using red_t = reducer<val_pool, val_pool, env_pool, env_lookup>;
    using re_t =
        reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, val_pool>;
    using proc_t = processor<red_t, re_t>;
    using interp_t = interpreter<continuation, proc_t, proc_t>;

    bool normalize_with_step_limit(const expr*& out, const expr* term,
                                   uint64_t max_steps) {
        env_pool envs;
        val_pool vals;
        env_lookup lookup;
        red_t red{vals, vals, envs, lookup};
        re_t re{pool, pool, pool, vals, envs, vals};
        proc_t proc{red, re};
        normalizer<val_pool> norm{vals};
        interp_t interp{proc, proc, norm.normalize(out, term)};
        for(uint64_t i = 0; i < max_steps && !interp.done(); ++i)
            interp.step();
        return interp.done();
    }

    void run_normalize(const expr*& out, const expr* term) {
        env_pool envs;
        val_pool vals;
        env_lookup lookup;
        red_t red{vals, vals, envs, lookup};
        re_t re{pool, pool, pool, vals, envs, vals};
        proc_t proc{red, re};
        normalizer<val_pool> norm{vals};
        interp_t interp{proc, proc, norm.normalize(out, term)};
        while(!interp.done())
            interp.step();
        DEBUG_ASSERT(interp.done());
    }

    const expr* normalize(const expr* term) {
        const expr* out;
        run_normalize(out, term);
        return out;
    }

    expr_pool pool;
};

#endif
