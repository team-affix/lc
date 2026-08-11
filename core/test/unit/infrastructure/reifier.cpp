#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_after_whnf_stage.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/val.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeVar {
    MOCK_METHOD(std::shared_ptr<expr>, make_var, (uint32_t), ());
};

struct MockMakeAbs {
    MOCK_METHOD(std::shared_ptr<expr>, make_abs, (std::shared_ptr<expr>), ());
};

struct MockMakeApp {
    MOCK_METHOD(std::shared_ptr<expr>, make_app,
                (std::shared_ptr<expr>, std::shared_ptr<expr>), ());
};

struct MockMakeFvar {
    MOCK_METHOD(std::shared_ptr<val>, make_fvar, (uint32_t), ());
};

struct MockMakeEnv {
    MOCK_METHOD(std::shared_ptr<env>, make_env,
                (std::shared_ptr<val>, std::shared_ptr<env>), ());
};

struct MockMakeClo {
    MOCK_METHOD(std::shared_ptr<val>, make_clo,
                (std::shared_ptr<expr>, std::shared_ptr<env>), ());
};

using test_reifier_t =
    reifier<MockMakeVar, MockMakeAbs, MockMakeApp, MockMakeFvar, MockMakeEnv,
            MockMakeClo>;

struct ReifierMockTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    expr_pool<rc_pool<expr>> pool;
    val_pool<rc_pool<val>> vals;
    NiceMock<MockMakeVar> make_var;
    NiceMock<MockMakeAbs> make_abs;
    NiceMock<MockMakeApp> make_app;
    NiceMock<MockMakeFvar> make_fvar;
    NiceMock<MockMakeEnv> make_env;
    NiceMock<MockMakeClo> make_clo;
    test_reifier_t re{make_var, make_abs, make_app, make_fvar, make_env,
                      make_clo};

    ReifierMockTest()
        : expr_nodes(), val_nodes(), pool(expr_nodes), vals(val_nodes) {
    }
};

TEST_F(ReifierMockTest, ProcessValOnFvarAfterWhnfWritesVar) {
    auto fv = vals.make_fvar(0);
    auto expected = pool.make_var(0);
    EXPECT_CALL(make_var, make_var(0)).WillOnce(Return(expected));
    std::shared_ptr<expr> got;
    reify_val_frame f{got, fv, 1};
    auto result = re.process(f, reify_val_after_whnf_stage{});
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(got, expected);
}

struct ReifierTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    rc_pool<env> env_nodes;
    expr_pool<rc_pool<expr>> pool;
    env_pool<rc_pool<env>> envs;
    val_pool<rc_pool<val>> vals;
    env_lookup lookup;
    reducer<val_pool<rc_pool<val>>, val_pool<rc_pool<val>>,
            env_pool<rc_pool<env>>, env_lookup>
        red;
    reifier<expr_pool<rc_pool<expr>>, expr_pool<rc_pool<expr>>,
            expr_pool<rc_pool<expr>>, val_pool<rc_pool<val>>,
            env_pool<rc_pool<env>>, val_pool<rc_pool<val>>>
        re;
    processor<decltype(red), decltype(re)> proc;

    ReifierTest()
        : expr_nodes()
        , val_nodes()
        , env_nodes()
        , pool(expr_nodes)
        , envs(env_nodes)
        , vals(val_nodes)
        , lookup()
        , red(vals, vals, envs, lookup)
        , re(pool, pool, pool, vals, envs, vals)
        , proc(red, re) {
    }

    ~ReifierTest() {
        garbage_collector<rc_pool<expr>, rc_pool<val>, rc_pool<env>> gc{
            expr_nodes, val_nodes, env_nodes};
        gc.collect();
    }

    std::shared_ptr<expr> dv(uint32_t i) { return pool.make_var(i); }
    std::shared_ptr<expr> lm(std::shared_ptr<expr> b) {
        return pool.make_abs(std::move(b));
    }
    std::shared_ptr<expr> ap(std::shared_ptr<expr> f, std::shared_ptr<expr> a) {
        return pool.make_app(std::move(f), std::move(a));
    }

    std::shared_ptr<expr> must_reify(std::shared_ptr<val> v, uint32_t depth) {
        std::shared_ptr<expr> e;
        interpreter<continuation, decltype(proc), decltype(proc)> interp{
            proc, proc, reify_val_funcall{e, std::move(v), depth}};
        while(!interp.done())
            interp.step();
        EXPECT_TRUE(interp.done());
        return e;
    }
};

TEST_F(ReifierTest, ReifyClosureAtLevelZeroGivesId) {
    auto id = lm(dv(0));
    auto clo = vals.make_clo(id, {});
    EXPECT_TRUE(exprs_eq(must_reify(clo, 0), id));
}

TEST_F(ReifierTest, ReifyFvarAtImmediateLevel) {
    auto fv = vals.make_fvar(0);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 1), dv(0)));
}

TEST_F(ReifierTest, ReifyFvarAtHigherLevel) {
    auto fv = vals.make_fvar(0);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 2), dv(1)));
}

TEST_F(ReifierTest, ReifyInnerFvarAtHigherLevel) {
    auto fv = vals.make_fvar(1);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 2), dv(0)));
}

TEST_F(ReifierTest, ReifyNappGivesApp) {
    auto head = vals.make_fvar(0);
    auto arg = lm(dv(0));
    auto n = vals.make_napp(head, arg, {});
    EXPECT_TRUE(exprs_eq(must_reify(n, 1), ap(dv(0), lm(dv(0)))));
}

TEST_F(ReifierTest, ReifyNestedNappGivesNestedApp) {
    auto head = vals.make_fvar(0);
    auto id = lm(dv(0));
    auto inner = vals.make_napp(head, id, {});
    auto outer = vals.make_napp(inner, id, {});
    EXPECT_TRUE(exprs_eq(must_reify(outer, 1), ap(ap(dv(0), id), id)));
}

TEST_F(ReifierTest, ReifySeededCloNormalizesIdentityApp) {
    std::shared_ptr<expr> out;
    auto seed = vals.make_clo(ap(lm(dv(0)), lm(dv(0))), {});
    interpreter<continuation, decltype(proc), decltype(proc)> interp{
        proc, proc, reify_val_funcall{out, seed, 0}};
    while(!interp.done())
        interp.step();
    ASSERT_TRUE(interp.done());
    EXPECT_TRUE(exprs_eq(out, lm(dv(0))));
}
