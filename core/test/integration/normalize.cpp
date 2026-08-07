#include <gtest/gtest.h>
#include "exprs_eq.hpp"
#include "nbe_fixture.hpp"

struct NormalizeIntegrationTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizeIntegrationTest, SelfApplicationOfIdentity) {
    EXPECT_TRUE(exprs_eq(normalize(ap(lm(ap(dv(0), dv(0))), id_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, KAppliedToTwoIdentities) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(k_term(), id_term()), id_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, KAppliedToIdAndK) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(k_term(), id_term()), k_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, KFirstArgReturnedAsAbs) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(k_term(), k_term()), id_term())),
                         k_term()));
}

TEST_F(NormalizeIntegrationTest, SKKIsIdentity) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(ap(s_term(), k_term()), k_term()), id_term())),
        id_term()));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccZeroEqualsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(succ_comb(), church(0))), church(1)));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccOneEqualsTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(succ_comb(), church(1))), church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccTwoEqualsThree) {
    EXPECT_TRUE(exprs_eq(normalize(ap(succ_comb(), church(2))), church(3)));
}

TEST_F(NormalizeIntegrationTest, ChurchSuccAppliedTwiceToZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(succ_comb(), ap(succ_comb(), church(0)))),
                         church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusOneOneEqualsTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(plus_comb(), church(1)), church(1))),
                         church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusTwoOneEqualsThree) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(plus_comb(), church(2)), church(1))),
                         church(3)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesOneTwoEqualsTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(1)), church(2))),
                         church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesTwoOneEqualsTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(2)), church(1))),
                         church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesTwoTwoEqualsFour) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(2)), church(2))),
                         church(4)));
}

TEST_F(NormalizeIntegrationTest, BoolAndTrueTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(and_comb(), true_comb()), true_comb())),
                         true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolAndTrueFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(and_comb(), true_comb()), false_comb())),
                         false_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolAndFalseTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(and_comb(), false_comb()), true_comb())),
                         false_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolAndFalseFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(and_comb(), false_comb()), false_comb())),
                         false_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolOrFalseFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(or_comb(), false_comb()), false_comb())),
                         false_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolOrFalseTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(or_comb(), false_comb()), true_comb())),
                         true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolOrTrueFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(or_comb(), true_comb()), false_comb())),
                         true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolOrTrueTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(or_comb(), true_comb()), true_comb())),
                         true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolNotTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(not_comb(), true_comb())), false_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolNotFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(not_comb(), false_comb())), true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolDoubleNegationTrue) {
    EXPECT_TRUE(exprs_eq(normalize(ap(not_comb(), ap(not_comb(), true_comb()))),
                         true_comb()));
}

TEST_F(NormalizeIntegrationTest, BoolDoubleNegationFalse) {
    EXPECT_TRUE(exprs_eq(normalize(ap(not_comb(), ap(not_comb(), false_comb()))),
                         false_comb()));
}

TEST_F(NormalizeIntegrationTest, BetaUnderAbsWithBoundVar) {
    EXPECT_TRUE(exprs_eq(normalize(lm(ap(lm(dv(1)), dv(0)))), id_term()));
}

TEST_F(NormalizeIntegrationTest, CapturedEnvDiffersFromAppEnv) {
    const expr* true_c = true_comb();
    const expr* id = id_term();
    const expr* inner = ap(lm(ap(dv(0), id)), lm(dv(1)));
    const expr* term = ap(lm(inner), true_c);
    EXPECT_TRUE(exprs_eq(normalize(term), true_c));
}

TEST_F(NormalizeIntegrationTest, SKKNormalizesToIdentity) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(s_term(), k_term()), k_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, EtaContractIx) {
    EXPECT_TRUE(exprs_eq(normalize(lm(ap(id_term(), dv(0)))), id_term()));
}

TEST_F(NormalizeIntegrationTest, SharedArgSelfApplication) {
    const expr* kii = ap(ap(k_term(), id_term()), id_term());
    EXPECT_TRUE(exprs_eq(normalize(ap(lm(ap(dv(0), dv(0))), kii)), id_term()));
}

TEST_F(NormalizeIntegrationTest, NestedCaptureUnderTwoLambdas) {
    const expr* true_c = true_comb();
    const expr* apply_g = lm(ap(dv(0), lm(dv(3))));
    const expr* pass_b = lm(ap(dv(0), dv(1)));
    const expr* mid = ap(lm(ap(apply_g, pass_b)), id_term());
    const expr* term = ap(lm(mid), true_c);
    EXPECT_TRUE(exprs_eq(normalize(term), true_c));
}

TEST_F(NormalizeIntegrationTest, NeutralAppUnderAbs) {
    const expr* term = lm(ap(dv(0), id_term()));
    EXPECT_TRUE(exprs_eq(normalize(term), term));
}

TEST_F(NormalizeIntegrationTest, WCombDuplicatesArg) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(w_term(), k_term()), id_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, ComposeBComb) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(b_term(), k_term()), id_term())),
                         k_term()));
}

TEST_F(NormalizeIntegrationTest, NormalizeKAndFalse) {
    EXPECT_TRUE(exprs_eq(normalize(k_term()), k_term()));
    EXPECT_TRUE(exprs_eq(normalize(false_comb()), false_comb()));
}

TEST_F(NormalizeIntegrationTest, ChurchPredOneIsZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(pred_comb(), church(1))), church(0)));
}

TEST_F(NormalizeIntegrationTest, ChurchIsZeroZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(iszero_comb(), church(0))), true_comb()));
}

TEST_F(NormalizeIntegrationTest, ChurchIsZeroOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(iszero_comb(), church(1))), false_comb()));
}

TEST_F(NormalizeIntegrationTest, ChurchIfThenElse) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(ap(if_comb(), true_comb()), id_term()), k_term())),
        id_term()));
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(ap(if_comb(), false_comb()), id_term()), k_term())),
        k_term()));
}
