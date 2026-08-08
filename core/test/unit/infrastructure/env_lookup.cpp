#include <gtest/gtest.h>
#include <cstdint>
#include <stdexcept>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/val_pool.hpp"

struct EnvLookupTest : public ::testing::Test {
    env_lookup lookup;
    env_pool envs;
    expr_pool pool;
    val_pool vals;
};

TEST_F(EnvLookupTest, IndexZeroReturnsSameCell) {
    env* e = envs.make_env(vals.make_clo(pool.make_var(0), nullptr), nullptr);
    EXPECT_EQ(lookup.lookup(e, 0), e);
}

TEST_F(EnvLookupTest, IndexOneReturnsParent) {
    env* outer =
        envs.make_env(vals.make_clo(pool.make_var(1), nullptr), nullptr);
    env* inner =
        envs.make_env(vals.make_clo(pool.make_var(0), nullptr), outer);
    EXPECT_EQ(lookup.lookup(inner, 1), outer);
}

TEST_F(EnvLookupTest, NullEnvThrows) {
    EXPECT_THROW(lookup.lookup(nullptr, 0), std::logic_error);
}

TEST_F(EnvLookupTest, MissingParentThrows) {
    env* e = envs.make_env(vals.make_clo(pool.make_var(0), nullptr), nullptr);
    EXPECT_THROW(lookup.lookup(e, 1), std::logic_error);
}
