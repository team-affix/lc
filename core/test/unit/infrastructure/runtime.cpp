#include <gtest/gtest.h>
#include <memory>
#include "exprs_eq.hpp"
#include "infrastructure/runtime.hpp"
#include "nbe_fixture.hpp"

struct RuntimeTest : public ::testing::Test, public nbe_fixture {};

TEST_F(RuntimeTest, IdentityNormalizesToItself) {
    std::shared_ptr<expr> out;
    nbe_runtime rt{out, id_term().get()};
    while(!rt.done())
        rt.step();
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
    out.reset();
}

TEST_F(RuntimeTest, IdentityAppNormalizesToIdentity) {
    std::shared_ptr<expr> out;
    nbe_runtime rt{out, ap(id_term(), id_term()).get()};
    while(!rt.done())
        rt.step();
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
    out.reset();
}

TEST_F(RuntimeTest, ZeroStepLimitLeavesOutUntouched) {
    std::shared_ptr<expr> sentinel = id_term();
    std::shared_ptr<expr> out = sentinel;
    nbe_runtime rt{out, ap(id_term(), id_term()).get()};
    EXPECT_FALSE(rt.done());
    EXPECT_EQ(out, sentinel);
    out.reset();
}

TEST_F(RuntimeTest, StackAllocatedAbsNormalizes) {
    auto body = std::make_shared<expr>(expr{expr::var{0}});
    auto term = std::make_shared<expr>(expr{expr::abs{body}});
    std::shared_ptr<expr> out;
    nbe_runtime rt{out, term.get()};
    while(!rt.done())
        rt.step();
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
    out.reset();
}

TEST_F(RuntimeTest, LimitedStepsOnOmegaLeavesOutUntouched) {
    std::shared_ptr<expr> sentinel = id_term();
    std::shared_ptr<expr> out = sentinel;
    nbe_runtime rt{out, omega_term().get()};
    for(uint64_t i = 0; i < 64 && !rt.done(); ++i)
        rt.step();
    EXPECT_FALSE(rt.done());
    EXPECT_EQ(out, sentinel);
    out.reset();
}

TEST_F(RuntimeTest, KIdDiscardsOmega) {
    std::shared_ptr<expr> out;
    auto term = ap(ap(k_term(), id_term()), omega_term());
    nbe_runtime rt{out, term.get()};
    while(!rt.done())
        rt.step();
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, id_term()));
    out.reset();
}

TEST_F(RuntimeTest, ChurchSuccZeroIsOne) {
    std::shared_ptr<expr> out;
    nbe_runtime rt{out, ap(succ_comb(), church(0)).get()};
    while(!rt.done())
        rt.step();
    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(exprs_eq(out, church(1)));
    out.reset();
}
