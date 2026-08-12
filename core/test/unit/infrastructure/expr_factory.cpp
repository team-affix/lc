#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/rc_pool.hpp"

struct ExprFactoryInfraTest : public ::testing::Test {
    rc_pool<expr> nodes;
    expr_factory<rc_pool<expr>> pool;

    ExprFactoryInfraTest() : nodes(), pool(nodes) {}
};

TEST_F(ExprFactoryInfraTest, NestedAbsAppStructuralEquality) {
    auto id_a = pool.make_abs(pool.make_var(0));
    auto id_b = pool.make_abs(pool.make_var(0));
    auto app_a = pool.make_app(id_a, id_a);
    auto app_b = pool.make_app(id_b, id_b);
    EXPECT_TRUE(exprs_eq(id_a, id_b));
    EXPECT_TRUE(exprs_eq(app_a, app_b));
}

TEST_F(ExprFactoryInfraTest, DropReclaimsWithoutGrowingStorage) {
    std::size_t baseline = nodes.space_usage();
    {
        auto a = pool.make_var(0);
        auto b = pool.make_var(1);
        (void)a;
        (void)b;
        EXPECT_GT(nodes.space_usage(), baseline);
    }
    EXPECT_EQ(nodes.space_usage(), baseline + 2 * sizeof(std::optional<expr>));
    auto reused_a = pool.make_var(2);
    auto reused_b = pool.make_var(3);
    EXPECT_EQ(nodes.space_usage(), baseline + 2 * sizeof(std::optional<expr>));
    (void)reused_a;
    (void)reused_b;
}
