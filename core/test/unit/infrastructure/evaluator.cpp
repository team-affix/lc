#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <variant>
#include "infrastructure/env_pool.hpp"
#include "infrastructure/evaluator.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, const env*), ());
};

struct MockMakeNapp {
    MOCK_METHOD(const val*, make_napp, (const val*, const expr*, const env*), ());
};

struct MockMakeDelayed {
    MOCK_METHOD(const env*, make_delayed, (const expr*, const env*, const env*), ());
};

using test_evaluator_t = evaluator<MockMakeClo, MockMakeNapp, MockMakeDelayed>;

struct EvaluatorMockTest : public ::testing::Test {
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockMakeNapp> make_napp;
    NiceMock<MockMakeDelayed> make_delayed;
    test_evaluator_t ev{make_clo, make_napp, make_delayed};
    expr_pool pool;
    val_pool vals;
};

TEST_F(EvaluatorMockTest, EvalAbsInNilGivesClosure) {
    const expr* body = pool.make_var(0);
    const expr* term = pool.make_abs(body);
    const val* expected = vals.make_clo(body, nullptr);
    EXPECT_CALL(make_clo, make_clo(body, nullptr)).WillOnce(Return(expected));
    EXPECT_EQ(ev.eval(term, nullptr), expected);
}

struct EvaluatorTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    evaluator<val_pool, val_pool, env_pool> ev{vals, vals, envs};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }
};

TEST_F(EvaluatorTest, EvalAbsInNilGivesClosure) {
    const expr* body = dv(0);
    const val* v = ev.eval(lm(body), nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->body, body);
    EXPECT_EQ(c->captured, nullptr);
}

TEST_F(EvaluatorTest, EvalVarZeroInEnvLookupsBinding) {
    const expr* arg = lm(dv(0));
    const env* e = envs.make_delayed(arg, nullptr, nullptr);
    const val* v = ev.eval(dv(0), e);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->body, dv(0));
}

TEST_F(EvaluatorTest, EvalVarOneInEnvSkipsToOuterBinding) {
    const expr* outer_arg = lm(dv(1));
    const expr* inner_arg = lm(dv(0));
    const env* outer = envs.make_delayed(outer_arg, nullptr, nullptr);
    const env* inner = envs.make_delayed(inner_arg, nullptr, outer);
    const val* v = ev.eval(dv(1), inner);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->body, dv(1));
}

TEST_F(EvaluatorTest, EvalAppBetaStep) {
    const expr* id = lm(dv(0));
    const val* v = ev.eval(ap(id, id), nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->body, dv(0));
    EXPECT_EQ(c->captured, nullptr);
}

TEST_F(EvaluatorTest, LookupMemoizesDelayedAsReady) {
    const expr* arg = lm(dv(0));
    const env* e = envs.make_delayed(arg, nullptr, nullptr);
    const val* first = ev.eval(dv(0), e);
    const env::ready* bound = std::get_if<env::ready>(&e->binder);
    ASSERT_NE(bound, nullptr);
    EXPECT_EQ(bound->value, first);
    const val* second = ev.eval(dv(0), e);
    EXPECT_EQ(second, first);
}

TEST_F(EvaluatorTest, EvalAppNeutralFvarHead) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    const env* e = envs.make_ready(head, nullptr);
    const val* v = ev.eval(ap(dv(0), id), e);
    const val::napp* n = std::get_if<val::napp>(&v->content);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->head, head);
    EXPECT_EQ(n->arg, id);
}

TEST_F(EvaluatorTest, EvalAppNappBuildsNestedNapp) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    const env* e = envs.make_ready(head, nullptr);
    const val* v = ev.eval(ap(ap(dv(0), id), id), e);
    const val::napp* outer = std::get_if<val::napp>(&v->content);
    ASSERT_NE(outer, nullptr);
    const val::napp* inner = std::get_if<val::napp>(&outer->head->content);
    ASSERT_NE(inner, nullptr);
}
