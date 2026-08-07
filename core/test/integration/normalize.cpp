#include <gtest/gtest.h>
#include "nbe_fixture.hpp"

struct NormalizeIntegrationTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizeIntegrationTest, SelfApplicationOfIdentity) {
    EXPECT_EQ(normalize(ap(lm(ap(dv(0), dv(0))), id_term())), id_term());
}

TEST_F(NormalizeIntegrationTest, KAppliedToTwoIdentities) {
    EXPECT_EQ(normalize(ap(ap(k_term(), id_term()), id_term())), id_term());
}

TEST_F(NormalizeIntegrationTest, KAppliedToIdAndK) {
    EXPECT_EQ(normalize(ap(ap(k_term(), id_term()), k_term())), id_term());
}

TEST_F(NormalizeIntegrationTest, KFirstArgReturnedAsAbs) {
    EXPECT_EQ(normalize(ap(ap(k_term(), k_term()), id_term())), k_term());
}

TEST_F(NormalizeIntegrationTest, SKKIsIdentity) {
    EXPECT_EQ(normalize(ap(ap(ap(s_term(), k_term()), k_term()), id_term())),
              id_term());
}

TEST_F(NormalizeIntegrationTest, ChurchSuccZeroEqualsOne) {
    EXPECT_EQ(normalize(ap(succ_comb(), church(0))), church(1));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccOneEqualsTwo) {
    EXPECT_EQ(normalize(ap(succ_comb(), church(1))), church(2));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccTwoEqualsThree) {
    EXPECT_EQ(normalize(ap(succ_comb(), church(2))), church(3));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccAppliedTwiceToZero) {
    EXPECT_EQ(normalize(ap(succ_comb(), ap(succ_comb(), church(0)))), church(2));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusOneOneEqualsTwo) {
    EXPECT_EQ(normalize(ap(ap(plus_comb(), church(1)), church(1))), church(2));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusTwoOneEqualsThree) {
    EXPECT_EQ(normalize(ap(ap(plus_comb(), church(2)), church(1))), church(3));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesOneTwoEqualsTwo) {
    EXPECT_EQ(normalize(ap(ap(times_comb(), church(1)), church(2))), church(2));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesTwoOneEqualsTwo) {
    EXPECT_EQ(normalize(ap(ap(times_comb(), church(2)), church(1))), church(2));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesTwoTwoEqualsFour) {
    EXPECT_EQ(normalize(ap(ap(times_comb(), church(2)), church(2))), church(4));
}

TEST_F(NormalizeIntegrationTest, BoolAndTrueTrue) {
    EXPECT_EQ(normalize(ap(ap(and_comb(), true_comb()), true_comb())), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolAndTrueFalse) {
    EXPECT_EQ(normalize(ap(ap(and_comb(), true_comb()), false_comb())),
              false_comb());
}

TEST_F(NormalizeIntegrationTest, BoolAndFalseTrue) {
    EXPECT_EQ(normalize(ap(ap(and_comb(), false_comb()), true_comb())),
              false_comb());
}

TEST_F(NormalizeIntegrationTest, BoolAndFalseFalse) {
    EXPECT_EQ(normalize(ap(ap(and_comb(), false_comb()), false_comb())),
              false_comb());
}

TEST_F(NormalizeIntegrationTest, BoolOrFalseFalse) {
    EXPECT_EQ(normalize(ap(ap(or_comb(), false_comb()), false_comb())),
              false_comb());
}

TEST_F(NormalizeIntegrationTest, BoolOrFalseTrue) {
    EXPECT_EQ(normalize(ap(ap(or_comb(), false_comb()), true_comb())), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolOrTrueFalse) {
    EXPECT_EQ(normalize(ap(ap(or_comb(), true_comb()), false_comb())), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolOrTrueTrue) {
    EXPECT_EQ(normalize(ap(ap(or_comb(), true_comb()), true_comb())), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolNotTrue) {
    EXPECT_EQ(normalize(ap(not_comb(), true_comb())), false_comb());
}

TEST_F(NormalizeIntegrationTest, BoolNotFalse) {
    EXPECT_EQ(normalize(ap(not_comb(), false_comb())), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolDoubleNegationTrue) {
    EXPECT_EQ(normalize(ap(not_comb(), ap(not_comb(), true_comb()))), true_comb());
}

TEST_F(NormalizeIntegrationTest, BoolDoubleNegationFalse) {
    EXPECT_EQ(normalize(ap(not_comb(), ap(not_comb(), false_comb()))),
              false_comb());
}

TEST_F(NormalizeIntegrationTest, BetaUnderAbsWithBoundVar) {
    EXPECT_EQ(normalize(lm(ap(lm(dv(1)), dv(0)))), id_term());
}

TEST_F(NormalizeIntegrationTest, CapturedEnvDiffersFromAppEnv) {
    // (λa. (λf. f id) (λx. a)) TRUE  ⇒  TRUE
    const expr* true_c = true_comb();
    const expr* id = id_term();
    const expr* inner = ap(lm(ap(dv(0), id)), lm(dv(1)));
    const expr* term = ap(lm(inner), true_c);
    EXPECT_EQ(normalize(term), true_c);
}
