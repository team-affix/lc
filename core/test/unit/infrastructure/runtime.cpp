#include "infrastructure/runtime.hpp"
#include "exprs_eq.hpp"
#include "nbe_fixture.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <memory>

struct RuntimeTest : public ::testing::Test, public nbe_fixture {
    static constexpr uint64_t k_gc_interval = 1024;
};

TEST_F(RuntimeTest, IdentityNormalizesToItself) {
    std::shared_ptr<expr> out;
    {
        runtime rt{id_term(), k_gc_interval};
        while(!rt.done())
            rt.step();
        EXPECT_TRUE(rt.done());
        out = rt.output();
        EXPECT_TRUE(exprs_eq(out, id_term()));
    }
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, IdentityAppNormalizesToIdentity) {
    std::shared_ptr<expr> out;
    {
        runtime rt{ap(id_term(), id_term()), k_gc_interval};
        while(!rt.done())
            rt.step();
        EXPECT_TRUE(rt.done());
        out = rt.output();
        EXPECT_TRUE(exprs_eq(out, id_term()));
    }
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, ZeroStepsLeavesIncomplete) {
    runtime rt{ap(id_term(), id_term()), k_gc_interval};
    EXPECT_FALSE(rt.done());
}

TEST_F(RuntimeTest, MakeSharedAbsNormalizes) {
    auto body = std::make_shared<expr>(expr{expr::var{0}});
    auto term = std::make_shared<expr>(expr{expr::abs{body}});
    std::shared_ptr<expr> out;
    {
        runtime rt{term, k_gc_interval};
        while(!rt.done())
            rt.step();
        EXPECT_TRUE(rt.done());
        out = rt.output();
        EXPECT_TRUE(exprs_eq(out, id_term()));
    }
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, LimitedStepsOnOmegaLeavesIncomplete) {
    runtime rt{omega_term(), k_gc_interval};
    for(uint64_t i = 0; i < 64 && !rt.done(); ++i)
        rt.step();
    EXPECT_FALSE(rt.done());
}

TEST_F(RuntimeTest, DestroyIncompleteRuntimeDoesNotThrow) {
    {
        runtime rt{omega_term(), 16};
        for(uint64_t i = 0; i < 256; ++i)
            rt.step();
        EXPECT_FALSE(rt.done());
    }
}

TEST_F(RuntimeTest, KIdDiscardsOmega) {
    std::shared_ptr<expr> out;
    auto term = ap(ap(k_term(), id_term()), omega_term());
    {
        runtime rt{term, k_gc_interval};
        while(!rt.done())
            rt.step();
        EXPECT_TRUE(rt.done());
        out = rt.output();
        EXPECT_TRUE(exprs_eq(out, id_term()));
    }
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(RuntimeTest, ChurchSuccZeroIsOne) {
    std::shared_ptr<expr> out;
    {
        runtime rt{ap(succ_comb(), church(0)), k_gc_interval};
        while(!rt.done())
            rt.step();
        EXPECT_TRUE(rt.done());
        out = rt.output();
        EXPECT_TRUE(exprs_eq(out, church(1)));
    }
    EXPECT_TRUE(exprs_eq(out, church(1)));
}
