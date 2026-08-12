#include <gtest/gtest.h>
#include <memory>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/output_detacher.hpp"
#include "infrastructure/rc_pool.hpp"
#include "value_objects/detach_expr_continuation.hpp"
#include "value_objects/detach_expr_funcall.hpp"

struct OutputDetacherTest : public ::testing::Test {
    rc_pool<expr> nodes;
    rc_pool<expr> out_nodes;
    expr_factory<rc_pool<expr>> pool;
    output_detacher<rc_pool<expr>> detacher;

    OutputDetacherTest()
        : nodes(), out_nodes(), pool(nodes), detacher(out_nodes) {}

    std::shared_ptr<expr> detach_copy(std::shared_ptr<expr> src) {
        std::shared_ptr<expr> out;
        using detach_continuation = std::variant<detach_expr_continuation>;
        using detach_funcall = std::variant<detach_expr_funcall>;
        using detacher_t = output_detacher<rc_pool<expr>>;
        interpreter<detach_continuation, detach_funcall, detacher_t, detacher_t>
            interp{detacher, detacher,
                   detach_expr_funcall{out, std::move(src)}};
        while(!interp.done())
            interp.step();
        EXPECT_TRUE(interp.done());
        return out;
    }
};

TEST_F(OutputDetacherTest, DetachCopiesPoolTreeOntoStandaloneSharedPtrs) {
    auto term = pool.make_abs(pool.make_app(pool.make_var(0), pool.make_var(1)));
    auto prepared = detach_copy(term);
    EXPECT_NE(prepared.get(), term.get());
    EXPECT_TRUE(exprs_eq(prepared, term));
    term.reset();
    EXPECT_TRUE(exprs_eq(prepared, detach_copy(prepared)));
}

TEST_F(OutputDetacherTest, DetachCopiesThroughAbsAndApp) {
    auto body = std::make_shared<expr>(expr{expr::var{0}});
    auto term = std::make_shared<expr>(expr{expr::abs{body}});
    auto prepared = detach_copy(term);
    EXPECT_NE(prepared.get(), term.get());
    EXPECT_NE(std::get<expr::abs>(prepared->content).body.get(), body.get());
    EXPECT_TRUE(exprs_eq(prepared, term));
}

TEST_F(OutputDetacherTest, DetachDeepAbsSpine) {
    std::shared_ptr<expr> term = pool.make_var(0);
    for(int i = 0; i < 10000; ++i)
        term = pool.make_abs(std::move(term));
    auto prepared = detach_copy(term);
    EXPECT_NE(prepared.get(), term.get());
    EXPECT_TRUE(exprs_eq(prepared, term));
}
