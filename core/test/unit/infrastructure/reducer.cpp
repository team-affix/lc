#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <deque>
#include <optional>
#include <stdexcept>
#include <variant>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/env.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/val.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

struct MockMakeNapp {
    MOCK_METHOD(const val*, make_napp, (const val*, const expr*, env*), ());
};

struct MockMakeEnv {
    MOCK_METHOD(env*, make_env, (const val*, env*), ());
};

struct MockLookup {
    MOCK_METHOD(env*, lookup, (env*, uint32_t), ());
};

using test_reducer_t =
    reducer<MockMakeClo, MockMakeNapp, MockMakeEnv, MockLookup>;

struct ReducerMockTest : public ::testing::Test {
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockMakeNapp> make_napp;
    NiceMock<MockMakeEnv> make_env;
    NiceMock<MockLookup> lookup;
    test_reducer_t red{make_clo, make_napp, make_env, lookup};
    expr_pool pool;
    val_pool vals;
};

TEST_F(ReducerMockTest, ProcessWhnfAbsCloFinishes) {
    const expr* term = pool.make_abs(pool.make_var(0));
    const val* clo = vals.make_clo(term, nullptr);
    const val* slot = clo;
    reduce_whnf_frame f{slot, reduce_whnf_stage::start};
    std::optional<frame> child = red.process_whnf(f);
    EXPECT_FALSE(child.has_value());
    EXPECT_EQ(slot, clo);
}

struct ReducerTest : public ::testing::Test {
    expr_pool pool;
    env_pool envs;
    val_pool vals;
    env_lookup lookup;
    reducer<val_pool, val_pool, env_pool, env_lookup> red{vals, vals, envs,
                                                         lookup};

    const expr* dv(uint32_t i) { return pool.make_var(i); }
    const expr* lm(const expr* b) { return pool.make_abs(b); }
    const expr* ap(const expr* f, const expr* a) { return pool.make_app(f, a); }

    std::optional<frame> process_top(frame& top) {
        if(reduce_whnf_frame* f = std::get_if<reduce_whnf_frame>(&top))
            return red.process_whnf(*f);
        if(reduce_app_frame* f = std::get_if<reduce_app_frame>(&top))
            return red.process_app(*f);
        return red.process_var(std::get<reduce_var_frame>(top));
    }

    const val* must_whnf(const val* v) {
        const val* reg = v;
        std::deque<frame> stack;
        stack.push_back(
            reduce_whnf_frame{reg, reduce_whnf_stage::start});
        while(!stack.empty()) {
            std::optional<frame> child = process_top(stack.back());
            if(child.has_value())
                stack.push_back(std::move(*child));
            else
                stack.pop_back();
        }
        return reg;
    }

    const val* must_whnf_term(const expr* term, env* e) {
        return must_whnf(vals.make_clo(term, e));
    }
};

TEST_F(ReducerTest, WhnfAbsInNilGivesClosure) {
    const expr* body = dv(0);
    const expr* term = lm(body);
    const val* v = must_whnf_term(term, nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, term);
    EXPECT_EQ(c->environment, nullptr);
}

TEST_F(ReducerTest, UnboundVarNullEnvThrows) {
    const val* v = vals.make_clo(dv(0), nullptr);
    EXPECT_THROW(must_whnf(v), std::logic_error);
}

TEST_F(ReducerTest, LookupPastNilParentThrows) {
    env* e = envs.make_env(vals.make_clo(lm(dv(0)), nullptr), nullptr);
    const val* v = vals.make_clo(dv(1), e);
    EXPECT_THROW(must_whnf(v), std::logic_error);
}

TEST_F(ReducerTest, WhnfVarZeroInEnvLookupsBinding) {
    const expr* arg = lm(dv(0));
    env* e = envs.make_env(vals.make_clo(arg, nullptr), nullptr);
    const val* v = must_whnf_term(dv(0), e);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, arg);
}

TEST_F(ReducerTest, WhnfVarOneInEnvSkipsToOuterBinding) {
    const expr* outer_arg = lm(dv(1));
    const expr* inner_arg = lm(dv(0));
    env* outer = envs.make_env(vals.make_clo(outer_arg, nullptr), nullptr);
    env* inner = envs.make_env(vals.make_clo(inner_arg, nullptr), outer);
    const val* v = must_whnf_term(dv(1), inner);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, outer_arg);
}

TEST_F(ReducerTest, WhnfAppBetaStep) {
    const expr* id = lm(dv(0));
    const val* v = must_whnf_term(ap(id, id), nullptr);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, id);
    EXPECT_EQ(c->environment, nullptr);
}

TEST_F(ReducerTest, LookupMemoizesBoundValue) {
    const expr* arg = lm(dv(0));
    env* e = envs.make_env(vals.make_clo(arg, nullptr), nullptr);
    const val* first = must_whnf_term(dv(0), e);
    EXPECT_EQ(e->bound_value, first);
    const val* second = must_whnf_term(dv(0), e);
    EXPECT_EQ(second, first);
}

TEST_F(ReducerTest, WhnfFvarEarlyReturn) {
    const val* fv = vals.make_fvar(0);
    EXPECT_EQ(must_whnf(fv), fv);
}

TEST_F(ReducerTest, WhnfAppNeutralFvarHead) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    env* e = envs.make_env(head, nullptr);
    const val* v = must_whnf_term(ap(dv(0), id), e);
    const val::napp* n = std::get_if<val::napp>(&v->content);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->head, head);
    EXPECT_EQ(n->arg, id);
}

TEST_F(ReducerTest, WhnfAppNappBuildsNestedNapp) {
    const expr* id = lm(dv(0));
    const val* head = vals.make_fvar(0);
    env* e = envs.make_env(head, nullptr);
    const val* v = must_whnf_term(ap(ap(dv(0), id), id), e);
    const val::napp* outer = std::get_if<val::napp>(&v->content);
    ASSERT_NE(outer, nullptr);
    const val::napp* inner = std::get_if<val::napp>(&outer->head->content);
    ASSERT_NE(inner, nullptr);
}
