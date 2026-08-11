#include <chrono>
#include <memory>
#include <cstdint>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include "exprs_eq.hpp"
#include "infrastructure/runtime.hpp"
#include "nbe_fixture.hpp"

// Exploratory runtime drives: count steps / wall time, assert prepared NF.

struct RuntimePlaygroundTest : public ::testing::Test, public nbe_fixture {
    static constexpr uint64_t k_gc_interval = 1024;

    struct drive {
        runtime rt;
        uint64_t steps;
        double ms;
        std::shared_ptr<expr> out;

        drive(std::shared_ptr<expr> term)
            : rt(std::move(term), k_gc_interval), steps(0), ms(0), out() {
            const auto t0 = std::chrono::steady_clock::now();
            while(!rt.done()) {
                rt.step();
                ++steps;
            }
            const auto t1 = std::chrono::steady_clock::now();
            ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            EXPECT_TRUE(rt.done());
            out = rt.output();
        }
    };

    void note(const char* label, const drive& d) {
        RecordProperty("label", label);
        RecordProperty("steps", std::to_string(d.steps));
        RecordProperty("ms", std::to_string(d.ms));
        std::cerr << "[playground] " << label << ": steps=" << d.steps
                  << "  ms=" << d.ms << '\n';
    }
};

TEST_F(RuntimePlaygroundTest, SKKIsIdentity) {
    // S K K → λx. x
    drive d{ap(ap(s_term(), k_term()), k_term())};
    note("S K K", d);
    EXPECT_TRUE(exprs_eq(d.out, id_term()));
}

TEST_F(RuntimePlaygroundTest, SKSIIsIdentity) {
    // S (K I) I → I
    std::shared_ptr<expr> ki = ap(k_term(), id_term());
    drive d{ap(ap(s_term(), ki), id_term())};
    note("S (K I) I", d);
    EXPECT_TRUE(exprs_eq(d.out, id_term()));
}

TEST_F(RuntimePlaygroundTest, ChurchPlusThreeFour) {
    drive d{ap(ap(plus_comb(), church(3)), church(4))};
    note("plus 3 4", d);
    EXPECT_TRUE(exprs_eq(d.out, church(7)));
}

TEST_F(RuntimePlaygroundTest, ChurchTimesFiveSix) {
    drive d{ap(ap(times_comb(), church(5)), church(6))};
    note("times 5 6", d);
    EXPECT_TRUE(exprs_eq(d.out, church(30)));
}

TEST_F(RuntimePlaygroundTest, ChurchExpTwoFive) {
    // 2^5 = 32
    drive d{ap(ap(exp_comb(), church(2)), church(5))};
    note("exp 2 5", d);
    EXPECT_TRUE(exprs_eq(d.out, church(32)));
}

TEST_F(RuntimePlaygroundTest, ChurchExpThreeFour) {
    // 3^4 = 81 — NF is a tall successor tower
    drive d{ap(ap(exp_comb(), church(3)), church(4))};
    note("exp 3 4", d);
    EXPECT_TRUE(exprs_eq(d.out, church(81)));
}

TEST_F(RuntimePlaygroundTest, PredOfSeven) {
    drive d{ap(pred_comb(), church(7))};
    note("pred 7", d);
    EXPECT_TRUE(exprs_eq(d.out, church(6)));
}

TEST_F(RuntimePlaygroundTest, NestedKDiscardsJunk) {
    // K (K I Ω) (succ Ω) → I
    std::shared_ptr<expr> keep = ap(ap(k_term(), id_term()), omega_term());
    std::shared_ptr<expr> junk = ap(succ_comb(), omega_term());
    drive d{ap(ap(k_term(), keep), junk)};
    note("K (K I Ω) (succ Ω)", d);
    EXPECT_TRUE(exprs_eq(d.out, id_term()));
}

TEST_F(RuntimePlaygroundTest, PairFstSndRoundtrip) {
    std::shared_ptr<expr> p = ap(ap(pair_comb(), church(2)), church(5));
    drive fst_d{ap(fst_comb(), p)};
    note("fst (pair 2 5)", fst_d);
    EXPECT_TRUE(exprs_eq(fst_d.out, church(2)));

    drive snd_d{ap(snd_comb(), p)};
    note("snd (pair 2 5)", snd_d);
    EXPECT_TRUE(exprs_eq(snd_d.out, church(5)));
}

TEST_F(RuntimePlaygroundTest, FactIterFive) {
    drive d{ap(fact_iter_comb(), church(5))};
    note("fact_iter 5", d);
    EXPECT_TRUE(exprs_eq(d.out, church(120)));
}

TEST_F(RuntimePlaygroundTest, FactIterSeven) {
    drive d{ap(fact_iter_comb(), church(7))};
    note("fact_iter 7", d);
    EXPECT_TRUE(exprs_eq(d.out, church(5040)));
}

TEST_F(RuntimePlaygroundTest, FibFive) {
    drive d{ap(fib_comb(), church(5))};
    note("fib 5", d);
    EXPECT_TRUE(exprs_eq(d.out, church(5)));
}

TEST_F(RuntimePlaygroundTest, FibSeven) {
    // fib 7 = 13
    drive d{ap(fib_comb(), church(7))};
    note("fib 7", d);
    EXPECT_TRUE(exprs_eq(d.out, church(13)));
}

TEST_F(RuntimePlaygroundTest, FactBinIterSix) {
    // Loop counter is Church; product is LE binary. Compare to NF of bin(720).
    drive d{ap(fact_bin_iter_comb(), church(6))};
    note("fact_bin_iter 6", d);
    EXPECT_TRUE(exprs_eq(d.out, normalize(bin(720))));
}

TEST_F(RuntimePlaygroundTest, BooleanXorViaChurch) {
    // (a ∧ ¬b) ∨ (¬a ∧ b) for a=T b=F → T
    std::shared_ptr<expr> a = true_comb();
    std::shared_ptr<expr> b = false_comb();
    std::shared_ptr<expr> left = ap(ap(and_comb(), a), ap(not_comb(), b));
    std::shared_ptr<expr> right = ap(ap(and_comb(), ap(not_comb(), a)), b);
    drive d{ap(ap(or_comb(), left), right)};
    note("xor T F", d);
    EXPECT_TRUE(exprs_eq(d.out, true_comb()));
}

TEST_F(RuntimePlaygroundTest, DeepISpine) {
    // I (I (I … x)) under λx — collapses to λx.x
    std::shared_ptr<expr> spine = dv(0);
    for(int i = 0; i < 8; ++i)
        spine = ap(id_term(), spine);
    drive d{lm(spine)};
    note("λx. I^8 x", d);
    EXPECT_TRUE(exprs_eq(d.out, id_term()));
}

TEST_F(RuntimePlaygroundTest, BCombinatorCompose) {
    // B f g x = f (g x);  B succ (plus 1) 2 → 4
    drive d{ap(ap(ap(b_term(), succ_comb()), ap(plus_comb(), church(1))),
               church(2))};
    note("B succ (plus 1) 2", d);
    EXPECT_TRUE(exprs_eq(d.out, church(4)));
}

TEST_F(RuntimePlaygroundTest, WCombinatorDuplicates) {
    // W f x = f x x;  W times 3 → 9
    drive d{ap(ap(w_term(), times_comb()), church(3))};
    note("W times 3", d);
    EXPECT_TRUE(exprs_eq(d.out, church(9)));
}
