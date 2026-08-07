#include <gtest/gtest.h>
#include "infrastructure/expr_pool.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/expr_hash.hpp"

struct ExprHashTest : public ::testing::Test {
    expr_hash hasher;
};

TEST_F(ExprHashTest, SameVarHashesEqual) {
    expr a{expr::var{0}};
    expr b{expr::var{0}};
    EXPECT_EQ(hasher(a), hasher(b));
}

TEST_F(ExprHashTest, DifferentVarsMayDiffer) {
    expr a{expr::var{0}};
    expr b{expr::var{1}};
    EXPECT_NE(hasher(a), hasher(b));
}

struct ExprPoolTest : public ::testing::Test {
    expr_pool pool;
};

TEST_F(ExprPoolTest, VarInternedTwiceReturnsSamePointer) {
    EXPECT_EQ(pool.make_var(0), pool.make_var(0));
}

TEST_F(ExprPoolTest, DifferentVarsReturnDifferentPointers) {
    EXPECT_NE(pool.make_var(0), pool.make_var(1));
}

TEST_F(ExprPoolTest, AbsInternedTwiceReturnsSamePointer) {
    const expr* body = pool.make_var(0);
    EXPECT_EQ(pool.make_abs(body), pool.make_abs(body));
}

TEST_F(ExprPoolTest, AppInternedTwiceReturnsSamePointer) {
    const expr* v0 = pool.make_var(0);
    const expr* v1 = pool.make_var(1);
    EXPECT_EQ(pool.make_app(v0, v1), pool.make_app(v0, v1));
}

TEST_F(ExprPoolTest, DifferentAppsReturnDifferentPointers) {
    const expr* v0 = pool.make_var(0);
    const expr* v1 = pool.make_var(1);
    EXPECT_NE(pool.make_app(v0, v1), pool.make_app(v1, v0));
}
