#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/evaluator.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

struct MockMakeNapp {
    MOCK_METHOD(const val*, make_napp, (const val*, const expr*, env*), ());
};

struct MockMakeDelayed {
    MOCK_METHOD(env*, make_delayed, (const expr*, env*, env*), ());
};

struct MockLookup {
    MOCK_METHOD(env*, lookup, (env*, uint32_t), ());
};

using test_evaluator_t =
    evaluator<MockMakeClo, MockMakeNapp, MockMakeDelayed, MockLookup>;

struct EvaluatorMockTest : public ::testing::Test {
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockMakeNapp> make_napp;
    NiceMock<MockMakeDelayed> make_delayed;
    NiceMock<MockLookup> lookup;
    test_evaluator_t ev{make_clo, make_napp, make_delayed, lookup};
    expr_pool pool;
    val_pool vals;
    uint64_t budget{std::numeric_limits<uint64_t>::max()};
};

TEST_F(EvaluatorMockTest, EvalAbsInNilGivesClosure) {
    const expr* body = pool.make_var(0);
    const expr* term = pool.make_abs(body);
    const val* expected = vals.make_clo(body, nullptr);
    EXPECT_CALL(make_clo, make_clo(body, nullptr)).WillOnce(Return(expected));
    const val* got;
    ASSERT_TRUE(ev.eval(got, term, nullptr, budget));
    EXPECT_EQ(got, expected);
}

TEST_F(EvaluatorMockTest, BetaWithZeroBudgetReturnsFalse) {
    const expr* body = pool.make_var(0);
    const expr* id = pool.make_abs(body);
    const val* clo = vals.make_clo(body, nullptr);
    EXPECT_CALL(make_clo, make_clo(body, nullptr)).WillOnce(Return(clo));
    uint64_t left = 0;
    const val* out;
    EXPECT_FALSE(ev.eval(out, pool.make_app(id, id), nullptr, left));
}

struct EvaluatorTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    env_lookup lookup;
    evaluator<val_pool, val_pool, env_pool, env_lookup> ev{vals, vals, envs,
                                                          lookup};
    uint64_t budget{std::numeric_limits<uint64_t>::max()};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }

    const val* must_eval(const expr* term, env* e) {
        const val* v;
        EXPECT_TRUE(ev.eval(v, term, e, budget));
        return v;
    }
};

TEST_F(EvaluatorTest, EvalAbsInNilGivesClosure) {
    const expr* body = dv(0);
    const val* v = must_eval(lm(body), nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->body, body);
    EXPECT_EQ(c->captured, nullptr);
}

TEST_F(EvaluatorTest, UnboundVarNullEnvThrows) {
    const val* out;
    EXPECT_THROW(ev.eval(out, dv(0), nullptr, budget), std::logic_error);
}

TEST_F(EvaluatorTest, LookupPastNilParentThrows) {
    env* e = envs.make_delayed(lm(dv(0)), nullptr, nullptr);
    const val* out;
    EXPECT_THROW(ev.eval(out, dv(1), e, budget), std::logic_error);
}

TEST_F(EvaluatorTest, EvalVarZeroInEnvLookupsBinding) {
    const expr* arg = lm(dv(0));
    env* e = envs.make_delayed(arg, nullptr, nullptr);
    const val* v = must_eval(dv(0), e);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(exprs_eq(c->body, dv(0)));
}

TEST_F(EvaluatorTest, EvalVarOneInEnvSkipsToOuterBinding) {
    const expr* outer_arg = lm(dv(1));
    const expr* inner_arg = lm(dv(0));
    env* outer = envs.make_delayed(outer_arg, nullptr, nullptr);
    env* inner = envs.make_delayed(inner_arg, nullptr, outer);
    const val* v = must_eval(dv(1), inner);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(exprs_eq(c->body, dv(1)));
}

TEST_F(EvaluatorTest, EvalAppBetaStep) {
    const expr* id = lm(dv(0));
    const val* v = must_eval(ap(id, id), nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_TRUE(exprs_eq(c->body, dv(0)));
    EXPECT_EQ(c->captured, nullptr);
}

TEST_F(EvaluatorTest, LookupMemoizesDelayedAsReady) {
    const expr* arg = lm(dv(0));
    env* e = envs.make_delayed(arg, nullptr, nullptr);
    const val* first = must_eval(dv(0), e);
    const env::ready* bound = std::get_if<env::ready>(&e->binder);
    ASSERT_NE(bound, nullptr);
    EXPECT_EQ(bound->value, first);
    const val* second = must_eval(dv(0), e);
    EXPECT_EQ(second, first);
}

TEST_F(EvaluatorTest, EvalAppNeutralFvarHead) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    env* e = envs.make_ready(head, nullptr);
    const val* v = must_eval(ap(dv(0), id), e);
    const val::napp* n = std::get_if<val::napp>(&v->content);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->head, head);
    EXPECT_EQ(n->arg, id);
}

TEST_F(EvaluatorTest, EvalAppNappBuildsNestedNapp) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    env* e = envs.make_ready(head, nullptr);
    const val* v = must_eval(ap(ap(dv(0), id), id), e);
    const val::napp* outer = std::get_if<val::napp>(&v->content);
    ASSERT_NE(outer, nullptr);
    const val::napp* inner = std::get_if<val::napp>(&outer->head->content);
    ASSERT_NE(inner, nullptr);
}
