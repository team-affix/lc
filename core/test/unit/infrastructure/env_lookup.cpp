#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/val_pool.hpp"

struct EnvLookupTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    rc_pool<env> env_nodes;
    expr_pool<rc_pool<expr>> pool;
    val_pool<rc_pool<val>> vals;
    env_pool<rc_pool<env>> envs;
    env_lookup lookup;

    EnvLookupTest()
        : expr_nodes()
        , val_nodes()
        , env_nodes()
        , pool(expr_nodes)
        , vals(val_nodes)
        , envs(env_nodes)
        , lookup() {
    }

    ~EnvLookupTest() {
        garbage_collector<rc_pool<expr>, rc_pool<val>, rc_pool<env>> gc{
            expr_nodes, val_nodes, env_nodes};
        gc.collect();
    }
};

TEST_F(EnvLookupTest, IndexZeroReturnsSameCell) {
    auto e = envs.make_env(vals.make_clo(pool.make_var(0), {}), {});
    EXPECT_EQ(lookup.lookup(e.get(), 0), e.get());
}

TEST_F(EnvLookupTest, IndexOneReturnsParent) {
    auto outer = envs.make_env(vals.make_clo(pool.make_var(1), {}), {});
    auto inner = envs.make_env(vals.make_clo(pool.make_var(0), {}), outer);
    EXPECT_EQ(lookup.lookup(inner.get(), 1), outer.get());
}

TEST_F(EnvLookupTest, NullEnvThrows) {
    EXPECT_THROW(lookup.lookup(nullptr, 0), std::logic_error);
}

TEST_F(EnvLookupTest, MissingParentThrows) {
    auto e = envs.make_env(vals.make_clo(pool.make_var(0), {}), {});
    EXPECT_THROW(lookup.lookup(e.get(), 1), std::logic_error);
}
