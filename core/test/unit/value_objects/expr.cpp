#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"
#include "value_objects/expr.hpp"

struct ExprPoolTest : public ::testing::Test {
    expr_pool pool;
};

TEST_F(ExprPoolTest, StructurallyEqualVarsCompareEqual) {
    const expr* a = pool.make_var(0);
    const expr* b = pool.make_var(0);
    EXPECT_NE(a, b);
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, DifferentVarsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_var(0), pool.make_var(1)));
}

TEST_F(ExprPoolTest, StructurallyEqualAbsCompareEqual) {
    const expr* a = pool.make_abs(pool.make_var(0));
    const expr* b = pool.make_abs(pool.make_var(0));
    EXPECT_NE(a, b);
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, StructurallyEqualAppsCompareEqual) {
    const expr* a = pool.make_app(pool.make_var(0), pool.make_var(1));
    const expr* b = pool.make_app(pool.make_var(0), pool.make_var(1));
    EXPECT_NE(a, b);
    EXPECT_TRUE(exprs_eq(a, b));
}

TEST_F(ExprPoolTest, DifferentAppsCompareUnequal) {
    EXPECT_FALSE(exprs_eq(pool.make_app(pool.make_var(0), pool.make_var(1)),
                          pool.make_app(pool.make_var(1), pool.make_var(0))));
}
