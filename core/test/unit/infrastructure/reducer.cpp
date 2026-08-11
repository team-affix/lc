#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_factory.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_factory.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/env.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/val.hpp"
#include "value_objects/whnf_start_stage.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeClo {
    MOCK_METHOD(std::shared_ptr<val>, make_clo,
                (std::shared_ptr<expr>, std::shared_ptr<env>), ());
};

struct MockMakeNapp {
    MOCK_METHOD(std::shared_ptr<val>, make_napp,
                (std::shared_ptr<val>, std::shared_ptr<expr>,
                 std::shared_ptr<env>),
                ());
};

struct MockMakeEnv {
    MOCK_METHOD(std::shared_ptr<env>, make_env,
                (std::shared_ptr<val>, std::shared_ptr<env>), ());
};

struct MockLookup {
    MOCK_METHOD(env*, lookup, (env*, uint32_t), ());
};

using test_reducer_t =
    reducer<MockMakeClo, MockMakeNapp, MockMakeEnv, MockLookup>;

struct ReducerMockTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    expr_factory<rc_pool<expr>> pool;
    val_factory<rc_pool<val>> vals;
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockMakeNapp> make_napp;
    NiceMock<MockMakeEnv> make_env;
    NiceMock<MockLookup> lookup;
    test_reducer_t red{make_clo, make_napp, make_env, lookup};

    ReducerMockTest()
        : expr_nodes(), val_nodes(), pool(expr_nodes), vals(val_nodes) {
    }
};

TEST_F(ReducerMockTest, ProcessWhnfAbsCloFinishes) {
    auto term = pool.make_abs(pool.make_var(0));
    auto clo = vals.make_clo(term, {});
    std::shared_ptr<val> slot = clo;
    reduce_whnf_frame f{slot};
    auto result = red.process(f, whnf_start_stage{});
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(slot, clo);
}

struct ReducerTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    rc_pool<env> env_nodes;
    expr_factory<rc_pool<expr>> pool;
    env_factory<rc_pool<env>> envs;
    val_factory<rc_pool<val>> vals;
    env_lookup lookup;
    reducer<val_factory<rc_pool<val>>, val_factory<rc_pool<val>>,
            env_factory<rc_pool<env>>, env_lookup>
        red;
    reifier<expr_factory<rc_pool<expr>>, expr_factory<rc_pool<expr>>,
            expr_factory<rc_pool<expr>>, val_factory<rc_pool<val>>,
            env_factory<rc_pool<env>>, val_factory<rc_pool<val>>>
        re;
    processor<decltype(red), decltype(re)> proc;

    ReducerTest()
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

    ~ReducerTest() {
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

    std::shared_ptr<val> must_whnf(std::shared_ptr<val> v) {
        std::shared_ptr<val> reg = std::move(v);
        interpreter<continuation, decltype(proc), decltype(proc)> interp{
            proc, proc, reduce_whnf_funcall{reg}};
        while(!interp.done())
            interp.step();
        EXPECT_TRUE(interp.done());
        return reg;
    }

    std::shared_ptr<val> must_whnf_term(std::shared_ptr<expr> term,
                                        std::shared_ptr<env> e) {
        return must_whnf(vals.make_clo(std::move(term), std::move(e)));
    }
};

TEST_F(ReducerTest, WhnfAbsInNilGivesClosure) {
    auto body = dv(0);
    auto term = lm(body);
    auto v = must_whnf_term(term, {});
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, term);
    EXPECT_EQ(c->environment, nullptr);
}

TEST_F(ReducerTest, UnboundVarNullEnvThrows) {
    auto v = vals.make_clo(dv(0), {});
    EXPECT_THROW(must_whnf(v), std::logic_error);
}

TEST_F(ReducerTest, LookupPastNilParentThrows) {
    auto e = envs.make_env(vals.make_clo(lm(dv(0)), {}), {});
    auto v = vals.make_clo(dv(1), e);
    EXPECT_THROW(must_whnf(v), std::logic_error);
}

TEST_F(ReducerTest, WhnfVarZeroInEnvLookupsBinding) {
    auto arg = lm(dv(0));
    auto e = envs.make_env(vals.make_clo(arg, {}), {});
    auto v = must_whnf_term(dv(0), e);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, arg);
}

TEST_F(ReducerTest, WhnfVarOneInEnvSkipsToOuterBinding) {
    auto outer_arg = lm(dv(1));
    auto inner_arg = lm(dv(0));
    auto outer = envs.make_env(vals.make_clo(outer_arg, {}), {});
    auto inner = envs.make_env(vals.make_clo(inner_arg, {}), outer);
    auto v = must_whnf_term(dv(1), inner);
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, outer_arg);
}

TEST_F(ReducerTest, WhnfAppBetaStep) {
    auto id = lm(dv(0));
    auto v = must_whnf_term(ap(id, id), {});
    const val::clo* c = std::get_if<val::clo>(&v->content);
    ASSERT_NE(c, nullptr);
    EXPECT_EQ(c->term, id);
    EXPECT_EQ(c->environment, nullptr);
}

TEST_F(ReducerTest, LookupMemoizesBoundValue) {
    auto arg = lm(dv(0));
    auto e = envs.make_env(vals.make_clo(arg, {}), {});
    auto first = must_whnf_term(dv(0), e);
    EXPECT_EQ(e->bound_value, first);
    auto second = must_whnf_term(dv(0), e);
    EXPECT_EQ(second, first);
}

TEST_F(ReducerTest, WhnfFvarEarlyReturn) {
    auto fv = vals.make_fvar(0);
    EXPECT_EQ(must_whnf(fv), fv);
}

TEST_F(ReducerTest, WhnfAppNeutralFvarHead) {
    auto id = lm(dv(0));
    auto head = vals.make_fvar(0);
    auto e = envs.make_env(head, {});
    auto v = must_whnf_term(ap(dv(0), id), e);
    const val::napp* n = std::get_if<val::napp>(&v->content);
    ASSERT_NE(n, nullptr);
    EXPECT_EQ(n->head, head);
    EXPECT_EQ(n->arg, id);
}

TEST_F(ReducerTest, WhnfAppNappBuildsNestedNapp) {
    auto id = lm(dv(0));
    auto head = vals.make_fvar(0);
    auto e = envs.make_env(head, {});
    auto v = must_whnf_term(ap(ap(dv(0), id), id), e);
    const val::napp* outer = std::get_if<val::napp>(&v->content);
    ASSERT_NE(outer, nullptr);
    const val::napp* inner = std::get_if<val::napp>(&outer->head->content);
    ASSERT_NE(inner, nullptr);
}
