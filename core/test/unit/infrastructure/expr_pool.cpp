#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"

struct ExprPoolInfraTest : public ::testing::Test {
    expr_pool pool;
};

TEST_F(ExprPoolInfraTest, NestedAbsAppStructuralEquality) {
    const expr* id_a = pool.make_abs(pool.make_var(0));
    const expr* id_b = pool.make_abs(pool.make_var(0));
    const expr* app_a = pool.make_app(id_a, id_a);
    const expr* app_b = pool.make_app(id_b, id_b);
    EXPECT_TRUE(exprs_eq(id_a, id_b));
    EXPECT_TRUE(exprs_eq(app_a, app_b));
}
