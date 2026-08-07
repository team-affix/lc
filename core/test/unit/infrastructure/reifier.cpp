#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <limits>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::_;

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

struct MockMakeFrame {
    MOCK_METHOD(env*, make_frame, (const val*, env*), ());
};

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

struct MockWhnf {
    MOCK_METHOD(bool, whnf, (const val*&, const val*, uint64_t&), ());
};

using test_reifier_t =
    reifier<MockMakeVar, MockMakeAbs, MockMakeApp, MockMakeFvar, MockMakeFrame,
            MockMakeClo, MockWhnf>;

struct ReifierMockTest : public ::testing::Test {
    NiceMock<MockMakeVar> make_var;
    NiceMock<MockMakeAbs> make_abs;
    NiceMock<MockMakeApp> make_app;
    NiceMock<MockMakeFvar> make_fvar;
    NiceMock<MockMakeFrame> make_frame;
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockWhnf> whnf;
    test_reifier_t re{make_var, make_abs, make_app, make_fvar, make_frame,
                      make_clo, whnf};
    expr_pool pool;
    val_pool vals;
    uint64_t budget{std::numeric_limits<uint64_t>::max()};
};

TEST_F(ReifierMockTest, ReifyFvarAtImmediateLevel) {
    const val* fv = vals.make_fvar(0);
    const expr* expected = pool.make_var(0);
    EXPECT_CALL(make_var, make_var(0)).WillOnce(Return(expected));
    const expr* got;
    ASSERT_TRUE(re.reify(got, fv, 1, budget));
    EXPECT_EQ(got, expected);
}

TEST_F(ReifierMockTest, ReifyCloYieldsAbsOfQuotedBody) {
    const expr* body = pool.make_var(0);
    const expr* abs_term = pool.make_abs(body);
    val clo{val::clo{abs_term, nullptr}};
    val fresh{val::fvar{0}};
    env extended{&fresh, nullptr};
    const val* body_clo = vals.make_clo(body, &extended);
    const expr* quoted_var = pool.make_var(0);
    const expr* expected_abs = pool.make_abs(quoted_var);

    EXPECT_CALL(make_fvar, make_fvar(0)).WillOnce(Return(&fresh));
    EXPECT_CALL(make_frame, make_frame(&fresh, nullptr))
        .WillOnce(Return(&extended));
    EXPECT_CALL(make_clo, make_clo(body, &extended)).WillOnce(Return(body_clo));
    EXPECT_CALL(whnf, whnf(_, body_clo, _))
        .WillOnce(DoAll(SetArgReferee<0>(&fresh), Return(true)));
    EXPECT_CALL(make_var, make_var(0)).WillOnce(Return(quoted_var));
    EXPECT_CALL(make_abs, make_abs(quoted_var)).WillOnce(Return(expected_abs));

    const expr* got;
    ASSERT_TRUE(re.reify(got, &clo, 0, budget));
    EXPECT_EQ(got, expected_abs);
}

TEST_F(ReifierMockTest, ReifyNappYieldsApp) {
    val head{val::fvar{0}};
    const expr* arg_term = pool.make_var(0);
    val napp_v{val::napp{&head, arg_term, nullptr}};
    const val* arg_clo = vals.make_clo(arg_term, nullptr);
    val arg_whnf{val::fvar{0}};
    const expr* fun_nf = pool.make_var(0);
    const expr* arg_nf = pool.make_var(0);
    const expr* expected = pool.make_app(fun_nf, arg_nf);

    EXPECT_CALL(make_var, make_var(0))
        .WillOnce(Return(fun_nf))
        .WillOnce(Return(arg_nf));
    EXPECT_CALL(make_clo, make_clo(arg_term, nullptr)).WillOnce(Return(arg_clo));
    EXPECT_CALL(whnf, whnf(_, arg_clo, _))
        .WillOnce(DoAll(SetArgReferee<0>(&arg_whnf), Return(true)));
    EXPECT_CALL(make_app, make_app(fun_nf, arg_nf)).WillOnce(Return(expected));

    const expr* got;
    ASSERT_TRUE(re.reify(got, &napp_v, 1, budget));
    EXPECT_EQ(got, expected);
}

struct ReifierTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    env_lookup lookup;
    reducer<val_pool, val_pool, env_pool, env_lookup> red{vals, vals, envs,
                                                         lookup};
    reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, val_pool,
            reducer<val_pool, val_pool, env_pool, env_lookup>>
        re{pool, pool, pool, vals, envs, vals, red};
    uint64_t budget{std::numeric_limits<uint64_t>::max()};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }

    const expr* must_reify(const val* v, uint32_t depth) {
        const expr* e;
        EXPECT_TRUE(re.reify(e, v, depth, budget));
        return e;
    }
};

TEST_F(ReifierTest, ReifyClosureAtLevelZeroGivesId) {
    const expr* id = lm(dv(0));
    const val* clo = vals.make_clo(id, nullptr);
    EXPECT_TRUE(exprs_eq(must_reify(clo, 0), id));
}

TEST_F(ReifierTest, ReifyFvarAtImmediateLevel) {
    const val* fv = vals.make_fvar(0);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 1), dv(0)));
}

TEST_F(ReifierTest, ReifyFvarAtHigherLevel) {
    const val* fv = vals.make_fvar(0);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 2), dv(1)));
}

TEST_F(ReifierTest, ReifyInnerFvarAtHigherLevel) {
    const val* fv = vals.make_fvar(1);
    EXPECT_TRUE(exprs_eq(must_reify(fv, 2), dv(0)));
}

TEST_F(ReifierTest, ReifyNappGivesApp) {
    const val* head = vals.make_fvar(0);
    const expr* arg = lm(dv(0));
    const val* n = vals.make_napp(head, arg, nullptr);
    EXPECT_TRUE(exprs_eq(must_reify(n, 1), ap(dv(0), lm(dv(0)))));
}

TEST_F(ReifierTest, ReifyNestedNappGivesNestedApp) {
    const val* head = vals.make_fvar(0);
    const expr* id = lm(dv(0));
    const val* inner = vals.make_napp(head, id, nullptr);
    const val* outer = vals.make_napp(inner, id, nullptr);
    EXPECT_TRUE(exprs_eq(must_reify(outer, 1), ap(ap(dv(0), id), id)));
}

TEST_F(ReifierTest, ReifyTermNormalizesIdentityApp) {
    const expr* out;
    ASSERT_TRUE(re.reify_term(out, ap(lm(dv(0)), lm(dv(0))), budget));
    EXPECT_TRUE(exprs_eq(out, lm(dv(0))));
}
