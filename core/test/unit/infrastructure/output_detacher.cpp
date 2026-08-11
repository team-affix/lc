#include <gtest/gtest.h>
#include <memory>
#include "exprs_eq.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/output_detacher.hpp"
#include "infrastructure/rc_pool.hpp"

struct OutputDetacherTest : public ::testing::Test {
    rc_pool<expr> nodes;
    expr_factory<rc_pool<expr>> pool;
    output_detacher detacher;

    OutputDetacherTest() : nodes(), pool(nodes), detacher() {}
};

TEST_F(OutputDetacherTest, PrepareCopiesPoolTreeOntoStandaloneSharedPtrs) {
    auto term = pool.make_abs(pool.make_app(pool.make_var(0), pool.make_var(1)));
    auto prepared = detacher.prepare(term);
    EXPECT_NE(prepared.get(), term.get());
    EXPECT_TRUE(exprs_eq(prepared, term));
    term.reset();
    EXPECT_TRUE(exprs_eq(prepared, detacher.prepare(prepared)));
}

TEST_F(OutputDetacherTest, PrepareIsRecursiveThroughAbsAndApp) {
    auto body = std::make_shared<expr>(expr{expr::var{0}});
    auto term = std::make_shared<expr>(expr{expr::abs{body}});
    auto prepared = detacher.prepare(term);
    EXPECT_NE(prepared.get(), term.get());
    EXPECT_NE(std::get<expr::abs>(prepared->content).body.get(), body.get());
    EXPECT_TRUE(exprs_eq(prepared, term));
}
