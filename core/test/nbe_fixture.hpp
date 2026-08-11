#ifndef NBE_FIXTURE_HPP
#define NBE_FIXTURE_HPP

#include "debug_assert.hpp"
#include "infrastructure/entrypoint_processor.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_factory.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/initial_frame_generator.hpp"
#include "infrastructure/output_detacher.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/reduction_processor.hpp"
#include "infrastructure/reification_processor.hpp"
#include "infrastructure/val_factory.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/funcall.hpp"
#include <cstdint>
#include <memory>

struct nbe_fixture {
    using expr_nodes_t = rc_pool<expr>;
    using val_nodes_t = rc_pool<val>;
    using env_nodes_t = rc_pool<env>;
    using expr_factory_t = expr_factory<expr_nodes_t>;
    using val_factory_t = val_factory<val_nodes_t>;
    using env_factory_t = env_factory<env_nodes_t>;

    nbe_fixture() : expr_nodes(), pool(expr_nodes) {}

    std::shared_ptr<expr> dv(uint32_t index) { return pool.make_var(index); }
    std::shared_ptr<expr> lm(std::shared_ptr<expr> body) {
        return pool.make_abs(std::move(body));
    }
    std::shared_ptr<expr> ap(std::shared_ptr<expr> fun, std::shared_ptr<expr> arg) {
        return pool.make_app(std::move(fun), std::move(arg));
    }

    std::shared_ptr<expr> id_term() { return lm(dv(0)); }
    std::shared_ptr<expr> k_term() { return lm(lm(dv(1))); }
    std::shared_ptr<expr> s_term() {
        return lm(lm(lm(ap(ap(dv(2), dv(0)), ap(dv(1), dv(0))))));
    }

    std::shared_ptr<expr> church(uint32_t n) {
        std::shared_ptr<expr> body = dv(0);
        for(uint32_t i = 0; i < n; ++i)
            body = ap(dv(1), body);
        return lm(lm(body));
    }

    std::shared_ptr<expr> succ_comb() {
        return lm(lm(lm(ap(dv(1), ap(ap(dv(2), dv(1)), dv(0))))));
    }

    std::shared_ptr<expr> plus_comb() {
        return lm(lm(lm(lm(ap(ap(dv(3), dv(1)), ap(ap(dv(2), dv(1)), dv(0)))))));
    }

    std::shared_ptr<expr> times_comb() {
        return lm(lm(lm(ap(dv(2), ap(dv(1), dv(0))))));
    }

    std::shared_ptr<expr> true_comb() { return lm(lm(dv(1))); }
    std::shared_ptr<expr> false_comb() { return lm(lm(dv(0))); }
    std::shared_ptr<expr> and_comb() { return lm(lm(ap(ap(dv(1), dv(0)), dv(1)))); }
    std::shared_ptr<expr> or_comb() { return lm(lm(ap(ap(dv(1), dv(1)), dv(0)))); }
    std::shared_ptr<expr> not_comb() {
        return lm(ap(ap(dv(0), false_comb()), true_comb()));
    }

    std::shared_ptr<expr> w_term() { return lm(lm(ap(ap(dv(1), dv(0)), dv(0)))); }

    std::shared_ptr<expr> b_term() {
        return lm(lm(lm(ap(dv(2), ap(dv(1), dv(0))))));
    }

    std::shared_ptr<expr> pred_comb() {
        std::shared_ptr<expr> step = lm(lm(ap(dv(0), ap(dv(1), dv(3)))));
        return lm(lm(lm(ap(ap(ap(dv(2), step), lm(dv(1))), lm(dv(0))))));
    }

    std::shared_ptr<expr> iszero_comb() {
        return lm(ap(ap(dv(0), lm(false_comb())), true_comb()));
    }

    std::shared_ptr<expr> if_comb() {
        return lm(lm(lm(ap(ap(dv(2), dv(1)), dv(0)))));
    }

    // Ω = (λx. x x)(λx. x x) — diverges under β; for non-strict / early-stop tests.
    std::shared_ptr<expr> omega_term() {
        std::shared_ptr<expr> xx = lm(ap(dv(0), dv(0)));
        return ap(xx, xx);
    }

    // Y = λf.(λx. f (x x)) (λx. f (x x))
    std::shared_ptr<expr> y_comb() {
        std::shared_ptr<expr> xx = lm(ap(dv(1), ap(dv(0), dv(0))));
        return lm(ap(xx, xx));
    }

    // exp = λm.λn. n m  (Church m^n)
    std::shared_ptr<expr> exp_comb() { return lm(lm(ap(dv(0), dv(1)))); }

    // pair = λa.λb.λf. f a b;  fst / snd project.
    std::shared_ptr<expr> pair_comb() {
        return lm(lm(lm(ap(ap(dv(0), dv(2)), dv(1)))));
    }
    std::shared_ptr<expr> fst_comb() { return lm(ap(dv(0), lm(lm(dv(1))))); }
    std::shared_ptr<expr> snd_comb() { return lm(ap(dv(0), lm(lm(dv(0))))); }

    // fact = Y (λr. λn. if (iszero n) 1 (times n (r (pred n))))
    std::shared_ptr<expr> fact_comb() {
        std::shared_ptr<expr> else_branch =
            ap(ap(times_comb(), dv(0)), ap(dv(1), ap(pred_comb(), dv(0))));
        std::shared_ptr<expr> body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(0))), church(1)),
               else_branch);
        return ap(y_comb(), lm(lm(body)));
    }

    // Iterative fact via Church n as a loop (no Y):
    //   fact n = snd (n (λp. (λk. pair k (× k (snd p))) (succ (fst p)))
    //                  (pair 0 1))
    std::shared_ptr<expr> fact_iter_comb() {
        // under λp λk: k=0, p=1
        std::shared_ptr<expr> with_k =
            ap(ap(pair_comb(), dv(0)),
               ap(ap(times_comb(), dv(0)), ap(snd_comb(), dv(1))));
        std::shared_ptr<expr> step =
            lm(ap(lm(with_k), ap(succ_comb(), ap(fst_comb(), dv(0)))));
        std::shared_ptr<expr> init =
            ap(ap(pair_comb(), church(0)), church(1));
        // under λn: n=0
        return lm(ap(snd_comb(), ap(ap(dv(0), step), init)));
    }

    // Little-endian binary: Church list of bits (true=1, false=0), LSB at head.
    // nil = λc.λn. n;  cons = λh.λt.λc.λn. c h t
    std::shared_ptr<expr> bin_nil() { return lm(lm(dv(0))); }
    std::shared_ptr<expr> bin_cons() {
        return lm(lm(lm(lm(ap(ap(dv(1), dv(3)), dv(2))))));
    }
    std::shared_ptr<expr> bin(uint32_t n) {
        if(n == 0)
            return bin_nil();
        std::shared_ptr<expr> bit = (n & 1u) != 0u ? true_comb() : false_comb();
        return ap(ap(bin_cons(), bit), bin(n >> 1));
    }

    // succ [] = [1];  succ (0:xs) = 1:xs;  succ (1:xs) = 0:(succ xs)
    std::shared_ptr<expr> bin_succ_comb() {
        // under λs λxs λb λbs: bs=0, b=1, xs=2, s=3
        std::shared_ptr<expr> on_cons = lm(lm(ap(
            ap(dv(1),
               ap(ap(bin_cons(), false_comb()), ap(dv(3), dv(0)))),
            ap(ap(bin_cons(), true_comb()), dv(0)))));
        std::shared_ptr<expr> body = lm(ap(ap(dv(0), on_cons),
                                 ap(ap(bin_cons(), true_comb()), bin_nil())));
        return ap(y_comb(), lm(body));
    }

    // dbl [] = [];  dbl xs = 0:xs  (×2)
    std::shared_ptr<expr> bin_dbl_comb() {
        return lm(ap(ap(dv(0), lm(lm(ap(ap(bin_cons(), false_comb()), dv(2))))),
                     bin_nil()));
    }

    std::shared_ptr<expr> bin_xor_comb() {
        // λa.λb. a (not b) b
        return lm(lm(ap(ap(dv(1), ap(not_comb(), dv(0))), dv(0))));
    }
    std::shared_ptr<expr> bin_maj_comb() {
        // λx.λy.λc. (x∧y) ∨ (x∧c) ∨ (y∧c)
        std::shared_ptr<expr> xy = ap(ap(and_comb(), dv(2)), dv(1));
        std::shared_ptr<expr> xc = ap(ap(and_comb(), dv(2)), dv(0));
        std::shared_ptr<expr> yc = ap(ap(and_comb(), dv(1)), dv(0));
        return lm(lm(lm(ap(ap(or_comb(), xy), ap(ap(or_comb(), xc), yc)))));
    }

    // addc xs ys cin — schoolbook add, bits LE
    std::shared_ptr<expr> bin_addc_comb() {
        // under λr λxs λys λcin λx λxs' λy λys':
        //   ys'=0 y=1 xs'=2 x=3 cin=4 ys=5 xs=6 r=7
        std::shared_ptr<expr> sum_bit = ap(
            ap(bin_xor_comb(), ap(ap(bin_xor_comb(), dv(3)), dv(1))), dv(4));
        std::shared_ptr<expr> cout =
            ap(ap(ap(bin_maj_comb(), dv(3)), dv(1)), dv(4));
        std::shared_ptr<expr> both = ap(
            ap(bin_cons(), sum_bit),
            ap(ap(ap(dv(7), dv(2)), dv(0)), cout));
        std::shared_ptr<expr> on_y = lm(lm(both));
        // under λr λxs λys λcin λx λxs': xs'=0 x=1 cin=2 ys=3 xs=4 r=5
        std::shared_ptr<expr> ys_nil = ap(
            ap(dv(2),
               ap(ap(dv(1),
                     ap(ap(bin_cons(), false_comb()),
                        ap(bin_succ_comb(), dv(0)))),
                  ap(ap(bin_cons(), true_comb()), dv(0)))),
            ap(ap(bin_cons(), dv(1)), dv(0)));
        std::shared_ptr<expr> on_x = lm(lm(ap(ap(dv(3), on_y), ys_nil)));
        // under λr λxs λys λcin: cin=0 ys=1 xs=2 r=3
        std::shared_ptr<expr> xs_nil_ys_nil = ap(
            ap(dv(0), ap(ap(bin_cons(), true_comb()), bin_nil())), bin_nil());
        // under λr λxs λys λcin λy λys': ys'=0 y=1 cin=2 ys=3 xs=4 r=5
        std::shared_ptr<expr> xs_nil_on_y = lm(lm(ap(
            ap(dv(2),
               ap(ap(dv(1),
                     ap(ap(bin_cons(), false_comb()),
                        ap(bin_succ_comb(), dv(0)))),
                  ap(ap(bin_cons(), true_comb()), dv(0)))),
            ap(ap(bin_cons(), dv(1)), dv(0)))));
        std::shared_ptr<expr> xs_nil =
            ap(ap(dv(1), xs_nil_on_y), xs_nil_ys_nil);
        // λxs λys λcin. xs on_x xs_nil
        std::shared_ptr<expr> body =
            lm(lm(lm(ap(ap(dv(2), on_x), xs_nil))));
        return ap(y_comb(), lm(body));
    }

    std::shared_ptr<expr> bin_add_comb() {
        return lm(lm(ap(ap(ap(bin_addc_comb(), dv(1)), dv(0)), false_comb())));
    }

    // mul xs ys: shift-and-add on LE bits
    std::shared_ptr<expr> bin_times_comb() {
        // under λm λxs λys λb λbs: bs=0 b=1 ys=2 xs=3 m=4
        std::shared_ptr<expr> shifted =
            ap(bin_dbl_comb(), ap(ap(dv(4), dv(0)), dv(2)));
        // under λm λxs λys λb λbs λp: p=0 bs=1 b=2 ys=3 xs=4 m=5
        std::shared_ptr<expr> with_p = ap(
            ap(dv(2), ap(ap(bin_add_comb(), dv(3)), dv(0))), dv(0));
        std::shared_ptr<expr> on_cons = lm(lm(ap(lm(with_p), shifted)));
        std::shared_ptr<expr> body =
            lm(lm(ap(ap(dv(1), on_cons), bin_nil()))); // λxs λys
        return ap(y_comb(), lm(body));
    }

    // Same iterative fact loop as fact_iter_comb, but counters/products are LE binary.
    std::shared_ptr<expr> fact_bin_iter_comb() {
        // under λp λk: k=0, p=1
        std::shared_ptr<expr> with_k =
            ap(ap(pair_comb(), dv(0)),
               ap(ap(bin_times_comb(), dv(0)), ap(snd_comb(), dv(1))));
        std::shared_ptr<expr> step = lm(
            ap(lm(with_k), ap(bin_succ_comb(), ap(fst_comb(), dv(0)))));
        std::shared_ptr<expr> init = ap(ap(pair_comb(), bin(0)), bin(1));
        return lm(ap(snd_comb(), ap(ap(dv(0), step), init)));
    }

    // Like fact_comb, but duplicates n into two distinct env cells so call-by-need
    // cannot share WHNF between the iszero/times use and the pred use:
    //   Y (λr. λn. (λn1. λn2. if (iszero n1) 1 (times n1 (r (pred n2)))) n n)
    std::shared_ptr<expr> fact_noshare_comb() {
        // under λr λn λn1 λn2: n2=0, n1=1, n=2, r=3
        std::shared_ptr<expr> else_branch =
            ap(ap(times_comb(), dv(1)), ap(dv(3), ap(pred_comb(), dv(0))));
        std::shared_ptr<expr> body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(1))), church(1)),
               else_branch);
        std::shared_ptr<expr> split = ap(ap(lm(lm(body)), dv(0)), dv(0));
        return ap(y_comb(), lm(lm(split)));
    }

    // fib = Y (λr. λn.
    //   if (iszero n) 0
    //   (if (iszero (pred n)) 1
    //    (plus (r (pred n)) (r (pred (pred n))))))
    std::shared_ptr<expr> fib_comb() {
        std::shared_ptr<expr> pred_n = ap(pred_comb(), dv(0));
        std::shared_ptr<expr> pred_pred_n = ap(pred_comb(), pred_n);
        std::shared_ptr<expr> else_branch =
            ap(ap(plus_comb(), ap(dv(1), pred_n)), ap(dv(1), pred_pred_n));
        std::shared_ptr<expr> inner_if =
            ap(ap(ap(if_comb(), ap(iszero_comb(), pred_n)), church(1)),
               else_branch);
        std::shared_ptr<expr> body =
            ap(ap(ap(if_comb(), ap(iszero_comb(), dv(0))), church(0)),
               inner_if);
        return ap(y_comb(), lm(lm(body)));
    }

    using red_t = reduction_processor<val_factory_t, val_factory_t, env_factory_t, env_lookup>;
    using re_t = reification_processor<expr_factory_t, expr_factory_t, expr_factory_t, val_factory_t,
                         env_factory_t, val_factory_t>;
    using proc_t = processor<red_t, re_t, entrypoint_processor, output_detacher>;
    using interp_t = interpreter<continuation, funcall, proc_t, proc_t>;

    bool normalize_with_step_limit(std::shared_ptr<expr>& out,
                                   std::shared_ptr<expr> term,
                                   uint64_t max_steps) {
        env_nodes_t env_nodes;
        val_nodes_t val_nodes;
        env_factory_t envs{env_nodes};
        val_factory_t vals{val_nodes};
        env_lookup lookup;
        red_t red{vals, vals, envs, lookup};
        re_t re{pool, pool, pool, vals, envs, vals};
        entrypoint_processor entrypoint;
        output_detacher detacher;
        proc_t proc{red, re, entrypoint, detacher};
        initial_frame_generator<val_factory_t> initial_frame_gen{vals};
        bool finished;
        {
            interp_t interp{proc, proc,
                            initial_frame_gen.generate_initial_frame(
                                out, std::move(term))};
            for(uint64_t i = 0; i < max_steps && !interp.done(); ++i)
                interp.step();
            finished = interp.done();
        }
        garbage_collector<expr_nodes_t, val_nodes_t, env_nodes_t> gc{
            expr_nodes, val_nodes, env_nodes};
        gc.collect();
        return finished;
    }

    void run_normalize(std::shared_ptr<expr>& out, std::shared_ptr<expr> term) {
        env_nodes_t env_nodes;
        val_nodes_t val_nodes;
        env_factory_t envs{env_nodes};
        val_factory_t vals{val_nodes};
        env_lookup lookup;
        red_t red{vals, vals, envs, lookup};
        re_t re{pool, pool, pool, vals, envs, vals};
        entrypoint_processor entrypoint;
        output_detacher detacher;
        proc_t proc{red, re, entrypoint, detacher};
        initial_frame_generator<val_factory_t> initial_frame_gen{vals};
        {
            interp_t interp{proc, proc,
                            initial_frame_gen.generate_initial_frame(
                                out, std::move(term))};
            while(!interp.done())
                interp.step();
            DEBUG_ASSERT(interp.done());
        }
        garbage_collector<expr_nodes_t, val_nodes_t, env_nodes_t> gc{
            expr_nodes, val_nodes, env_nodes};
        gc.collect();
    }

    std::shared_ptr<expr> normalize(std::shared_ptr<expr> term) {
        std::shared_ptr<expr> out;
        run_normalize(out, std::move(term));
        return out;
    }

    expr_nodes_t expr_nodes;
    expr_factory_t pool;
};

#endif
