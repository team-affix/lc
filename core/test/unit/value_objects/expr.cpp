#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"

struct ExprPoolTest : public ::testing::Test {
    rc_pool<expr> nodes;
    expr_pool<rc_pool<expr>> pool;

    ExprPoolTest() : nodes(), pool(nodes) {}
};

TEST_F(ExprPoolTest, StructurallyEqualVarsCompareEqual) {
    auto a = pool.make_var(0);
    auto b = pool.make_var(0);
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, DifferentVarsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_var(0), pool.make_var(1)));
}

TEST_F(ExprPoolTest, StructurallyEqualAbsCompareEqual) {
    auto a = pool.make_abs(pool.make_var(0));
    auto b = pool.make_abs(pool.make_var(0));
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, StructurallyEqualAppsCompareEqual) {
    auto a = pool.make_app(pool.make_var(0), pool.make_var(1));
    auto b = pool.make_app(pool.make_var(0), pool.make_var(1));
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, DifferentAppsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_app(pool.make_var(0), pool.make_var(1)),
                          pool.make_app(pool.make_var(1), pool.make_var(0))));
}
