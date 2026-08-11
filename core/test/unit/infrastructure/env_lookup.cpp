#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/val_pool.hpp"

struct EnvLookupTest : public ::testing::Test {
    env_lookup lookup;
    env_pool envs;
    expr_pool pool;
    val_pool vals;

    ~EnvLookupTest() {
        garbage_collector<expr_pool, val_pool, env_pool> gc{pool, vals, envs};
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
