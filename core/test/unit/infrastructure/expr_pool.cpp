#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/rc_pool.hpp"

struct ExprPoolInfraTest : public ::testing::Test {
    rc_pool<expr> nodes;
    expr_pool<rc_pool<expr>> pool;

    ExprPoolInfraTest() : nodes(), pool(nodes) {}
};

TEST_F(ExprPoolInfraTest, NestedAbsAppStructuralEquality) {
    auto id_a = pool.make_abs(pool.make_var(0));
    auto id_b = pool.make_abs(pool.make_var(0));
    auto app_a = pool.make_app(id_a, id_a);
    auto app_b = pool.make_app(id_b, id_b);
    EXPECT_TRUE(exprs_eq(id_a, id_b));
    EXPECT_TRUE(exprs_eq(app_a, app_b));
}

TEST_F(ExprPoolInfraTest, ImportCopiesHandBuiltTree) {
    auto body = std::make_shared<expr>(expr{expr::var{0}});
    auto term = std::make_shared<expr>(expr{expr::abs{body}});
    auto imported = pool.import(term.get());
    EXPECT_NE(imported.get(), term.get());
    EXPECT_TRUE(exprs_eq(imported, term));
}

TEST_F(ExprPoolInfraTest, CollectOneFalseWhenEmpty) {
    EXPECT_FALSE(nodes.collect_one());
}

TEST_F(ExprPoolInfraTest, ReleaseThenCollectOneRetiresSlot) {
    {
        auto a = pool.make_var(0);
        auto b = pool.make_var(1);
        (void)b;
    }
    EXPECT_TRUE(nodes.collect_one());
    EXPECT_TRUE(nodes.collect_one());
    EXPECT_FALSE(nodes.collect_one());
}
