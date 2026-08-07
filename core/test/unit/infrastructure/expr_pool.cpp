#include <gtest/gtest.h>
#include "infrastructure/expr_pool.hpp"

struct ExprPoolInfraTest : public ::testing::Test {
    expr_pool pool;
};

TEST_F(ExprPoolInfraTest, NestedAbsAppInterning) {
    const expr* id = pool.make_abs(pool.make_var(0));
    const expr* app = pool.make_app(id, id);
    EXPECT_EQ(pool.make_app(id, id), app);
}
