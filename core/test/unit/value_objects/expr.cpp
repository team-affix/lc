#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/rc_pool.hpp"
#include "value_objects/expr.hpp"

struct ExprFactoryTest : public ::testing::Test {
    rc_pool<expr> nodes;
    expr_factory<rc_pool<expr>> pool;

    ExprFactoryTest() : nodes(), pool(nodes) {}
};

TEST_F(ExprFactoryTest, StructurallyEqualVarsCompareEqual) {
    auto a = pool.make_var(0);
    auto b = pool.make_var(0);
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprFactoryTest, DifferentVarsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_var(0), pool.make_var(1)));
}

TEST_F(ExprFactoryTest, StructurallyEqualAbsCompareEqual) {
    auto a = pool.make_abs(pool.make_var(0));
    auto b = pool.make_abs(pool.make_var(0));
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprFactoryTest, StructurallyEqualAppsCompareEqual) {
    auto a = pool.make_app(pool.make_var(0), pool.make_var(1));
    auto b = pool.make_app(pool.make_var(0), pool.make_var(1));
    EXPECT_NE(a.get(), b.get());
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprFactoryTest, DifferentAppsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_app(pool.make_var(0), pool.make_var(1)),
                          pool.make_app(pool.make_var(1), pool.make_var(0))));
}
