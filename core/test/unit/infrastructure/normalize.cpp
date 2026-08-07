#include <gtest/gtest.h>
#include <cstdint>
#include <limits>
#include "exprs_eq.hpp"
#include "nbe_fixture.hpp"

struct NormalizeTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizeTest, NormalizeAbsVar) {
    EXPECT_TRUE(exprs_eq(normalize(lm(dv(0))), id_term()));
}

TEST_F(NormalizeTest, NormalizeIdentityOnIdentity) {
    EXPECT_TRUE(exprs_eq(normalize(ap(id_term(), id_term())), id_term()));
}

TEST_F(NormalizeTest, NormalizeExhaustsBetaBudget) {
    const expr* term = ap(id_term(), id_term());
    const expr* out;
    EXPECT_FALSE(normalize_with_budget(out, term, 0));
    ASSERT_TRUE(normalize_with_budget(out, term, 1));
    EXPECT_TRUE(exprs_eq(out, id_term()));
}
