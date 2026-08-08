#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <deque>
#include <optional>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_stage.hpp"
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

struct MockMakeEnv {
    MOCK_METHOD(env*, make_env, (const val*, env*), ());
};

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

using test_reifier_t =
    reifier<MockMakeVar, MockMakeAbs, MockMakeApp, MockMakeFvar, MockMakeEnv,
            MockMakeClo>;

struct ReifierMockTest : public ::testing::Test {
    NiceMock<MockMakeVar> make_var;
    NiceMock<MockMakeAbs> make_abs;
    NiceMock<MockMakeApp> make_app;
    NiceMock<MockMakeFvar> make_fvar;
    NiceMock<MockMakeEnv> make_env;
    NiceMock<MockMakeClo> make_clo;
    test_reifier_t re{make_var, make_abs, make_app, make_fvar, make_env,
                      make_clo};
    expr_pool pool;
    val_pool vals;
};

TEST_F(ReifierMockTest, ProcessValOnFvarAfterWhnfWritesVar) {
    const val* fv = vals.make_fvar(0);
    const expr* expected = pool.make_var(0);
    EXPECT_CALL(make_var, make_var(0)).WillOnce(Return(expected));
    const expr* got = nullptr;
    reify_val_frame f{got, fv, 1, reify_val_stage::after_whnf};
    std::optional<frame> child = re.process_val(f);
    EXPECT_FALSE(child.has_value());
    EXPECT_EQ(got, expected);
}

struct ReifierTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    env_lookup lookup;
    reducer<val_pool, val_pool, env_pool, env_lookup> red{vals, vals, envs,
                                                         lookup};
    reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool, val_pool> re{
        pool, pool, pool, vals, envs, vals};
    processor<reducer<val_pool, val_pool, env_pool, env_lookup>,
              reducer<val_pool, val_pool, env_pool, env_lookup>,
              reducer<val_pool, val_pool, env_pool, env_lookup>,
              reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool,
                      val_pool>,
              reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool,
                      val_pool>,
              reifier<expr_pool, expr_pool, expr_pool, val_pool, env_pool,
                      val_pool>>
        proc{red, red, red, re, re, re};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }

    const expr* must_reify(const val* v, uint32_t depth) {
        const expr* e = nullptr;
        interpreter<frame, decltype(proc)> interp{
            proc, reify_val_frame{e, v, depth, reify_val_stage::need_whnf}};
        while(interp.step()) {
        }
        EXPECT_TRUE(interp.done());
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

TEST_F(ReifierTest, ReifySeededCloNormalizesIdentityApp) {
    const expr* out = nullptr;
    const val* seed = vals.make_clo(ap(lm(dv(0)), lm(dv(0))), nullptr);
    interpreter<frame, decltype(proc)> interp{
        proc, reify_val_frame{out, seed, 0, reify_val_stage::need_whnf}};
    while(interp.step()) {
    }
    ASSERT_TRUE(interp.done());
    EXPECT_TRUE(exprs_eq(out, lm(dv(0))));
}
