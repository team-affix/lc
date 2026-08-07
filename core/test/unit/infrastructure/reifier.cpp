#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <variant>
#include "infrastructure/env_pool.hpp"
#include "infrastructure/evaluator.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/val.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeVar {
    MOCK_METHOD(const expr*, make_var, (uint32_t), ());
};

struct MockMakeAbs {
    MOCK_METHOD(const expr*, make_abs, (const expr*), ());
};

struct MockMakeApp {
    MOCK_METHOD(const expr*, make_app, (const expr*, const expr*), ());
};

struct MockMakeFvar {
    MOCK_METHOD(const val*, make_fvar, (uint32_t), ());
};

struct MockMakeReady {
    MOCK_METHOD(const env*, make_ready, (const val*, const env*), ());
};

struct MockEval {
    MOCK_METHOD(const val*, eval, (const expr*, const env*), ());
};

using test_reifier_t =
    reifier<MockMakeVar, MockMakeAbs, MockMakeApp, MockMakeFvar, MockMakeReady, MockEval>;

struct ReifierMockTest : public ::testing::Test {
    NiceMock<MockMakeVar> make_var;
    NiceMock<MockMakeAbs> make_abs;
    NiceMock<MockMakeApp> make_app;
    NiceMock<MockMakeFvar> make_fvar;
    NiceMock<MockMakeReady> make_ready;
    NiceMock<MockEval> eval;
    test_reifier_t re{make_var, make_abs, make_app, make_fvar, make_ready, eval};
    expr_pool pool;
    val_pool vals;
};

TEST_F(ReifierMockTest, ReifyFvarAtImmediateLevel) {
    const val* fv = vals.make_fvar(0);
    const expr* expected = pool.make_var(0);
    EXPECT_CALL(make_var, make_var(0)).WillOnce(Return(expected));
    EXPECT_EQ(re.reify(fv, 1), expected);
}

struct ReifierTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    evaluator<val_pool, val_pool, env_pool> ev{vals, vals, envs};
    reifier<expr_pool,
            expr_pool,
            expr_pool,
            val_pool,
            env_pool,
            evaluator<val_pool, val_pool, env_pool>>
        re{pool, pool, pool, vals, envs, ev};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }
};

TEST_F(ReifierTest, ReifyClosureAtLevelZeroGivesId) {
    const expr* body = dv(0);
    const val* clo = vals.make_clo(body, nullptr);
    EXPECT_EQ(re.reify(clo, 0), lm(dv(0)));
}

TEST_F(ReifierTest, ReifyFvarAtImmediateLevel) {
    const val* fv = vals.make_fvar(0);
    EXPECT_EQ(re.reify(fv, 1), dv(0));
}

TEST_F(ReifierTest, ReifyFvarAtHigherLevel) {
    const val* fv = vals.make_fvar(0);
    EXPECT_EQ(re.reify(fv, 2), dv(1));
}

TEST_F(ReifierTest, ReifyInnerFvarAtHigherLevel) {
    const val* fv = vals.make_fvar(1);
    EXPECT_EQ(re.reify(fv, 2), dv(0));
}

TEST_F(ReifierTest, ReifyNappGivesApp) {
    const val* head = vals.make_fvar(0);
    const expr* arg = lm(dv(0));
    const val* n = vals.make_napp(head, arg, nullptr);
    EXPECT_EQ(re.reify(n, 1), ap(dv(0), lm(dv(0))));
}

TEST_F(ReifierTest, ReifyNestedNappGivesNestedApp) {
    const val* head = vals.make_fvar(0);
    const expr* id = lm(dv(0));
    const val* inner = vals.make_napp(head, id, nullptr);
    const val* outer = vals.make_napp(inner, id, nullptr);
    EXPECT_EQ(re.reify(outer, 1), ap(ap(dv(0), id), id));
}
