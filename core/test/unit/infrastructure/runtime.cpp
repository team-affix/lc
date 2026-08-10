#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/runtime.hpp"
#include "nbe_fixture.hpp"

struct RuntimeTest : public ::testing::Test, public nbe_fixture {};

TEST_F(RuntimeTest, IdentityNormalizesToItself) {
    const expr* out = nullptr;
    runtime rt{out, id_term()};
    while(rt.step()) {
    }
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, IdentityAppNormalizesToIdentity) {
    const expr* out = nullptr;
    runtime rt{out, ap(id_term(), id_term())};
    while(rt.step()) {
    }
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, ZeroStepLimitLeavesOutUntouched) {
    const expr* sentinel = id_term();
    const expr* out = sentinel;
    runtime rt{out, ap(id_term(), id_term())};
    EXPECT_FALSE(rt.done());
    EXPECT_EQ(out, sentinel);
}

TEST_F(RuntimeTest, StackAllocatedAbsNormalizes) {
    expr body{expr::var{0}};
    expr term{expr::abs{&body}};
    const expr* out = nullptr;
    runtime rt{out, &term};
    while(rt.step()) {
    }
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, LimitedStepsOnOmegaLeavesOutUntouched) {
    const expr* sentinel = id_term();
    const expr* out = sentinel;
    runtime rt{out, omega_term()};
    for(uint64_t i = 0; i < 64; ++i) {
        if(!rt.step())
            break;
    }
    EXPECT_FALSE(rt.done());
    EXPECT_EQ(out, sentinel);
}

TEST_F(RuntimeTest, KIdDiscardsOmega) {
    const expr* out = nullptr;
    const expr* term = ap(ap(k_term(), id_term()), omega_term());
    runtime rt{out, term};
    while(rt.step()) {
    }
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, ChurchSuccZeroIsOne) {
    const expr* out = nullptr;
    runtime rt{out, ap(succ_comb(), church(0))};
    while(rt.step()) {
    }
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, church(1)));
}
