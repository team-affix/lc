#include <gtest/gtest.h>
#include <memory>
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
    std::shared_ptr<expr> true_c = true_comb();
    std::shared_ptr<expr> id = id_term();
    std::shared_ptr<expr> inner = ap(lm(ap(dv(0), id)), lm(dv(1)));
    std::shared_ptr<expr> term = ap(lm(inner), true_c);
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
    std::shared_ptr<expr> kii = ap(ap(k_term(), id_term()), id_term());
    EXPECT_TRUE(exprs_eq(normalize(ap(lm(ap(dv(0), dv(0))), kii)), id_term()));
}

TEST_F(NormalizeIntegrationTest, NestedCaptureUnderTwoLambdas) {
    std::shared_ptr<expr> true_c = true_comb();
    std::shared_ptr<expr> apply_g = lm(ap(dv(0), lm(dv(3))));
    std::shared_ptr<expr> pass_b = lm(ap(dv(0), dv(1)));
    std::shared_ptr<expr> mid = ap(lm(ap(apply_g, pass_b)), id_term());
    std::shared_ptr<expr> term = ap(lm(mid), true_c);
    EXPECT_TRUE(exprs_eq(normalize(term), true_c));
}

TEST_F(NormalizeIntegrationTest, NeutralAppUnderAbs) {
    std::shared_ptr<expr> term = lm(ap(dv(0), id_term()));
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

// ---------------------------------------------------------------------------
// Priority: non-strict discard (must not force Ω)
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, KIdDiscardsOmega) {
    // Call-by-value / force-at-β would exhaust any small budget on Ω.
    std::shared_ptr<expr> term = ap(ap(k_term(), id_term()), omega_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, KOmegaDiscardsOmegaKeepsConst) {
    std::shared_ptr<expr> term = ap(ap(k_term(), k_term()), omega_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, k_term()));
}

TEST_F(NormalizeIntegrationTest, FalseIfDiscardsThenOmega) {
    std::shared_ptr<expr> term =
        ap(ap(ap(if_comb(), false_comb()), omega_term()), id_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, TrueIfDiscardsElseOmega) {
    std::shared_ptr<expr> term =
        ap(ap(ap(if_comb(), true_comb()), id_term()), omega_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, NestedKDiscardOmegaUnderAbs) {
    // λz. K z Ω → λz. z
    std::shared_ptr<expr> term = lm(ap(ap(k_term(), dv(0)), omega_term()));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, ChurchFalseDiscardsSuccOmega) {
    // false A I → I without forcing A; A = succ Ω would diverge if forced.
    std::shared_ptr<expr> diverging = ap(succ_comb(), omega_term());
    std::shared_ptr<expr> term = ap(ap(false_comb(), diverging), id_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

// ---------------------------------------------------------------------------
// Priority: early stop / out untouched
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, OutUntouchedOnOmegaEarlyStop) {
    std::shared_ptr<expr> sentinel = id_term();
    std::shared_ptr<expr> out = sentinel;
    uint64_t steps = 10;
    EXPECT_FALSE(normalize_with_step_limit(out, omega_term(), steps));
    EXPECT_EQ(out, sentinel);
}

TEST_F(NormalizeIntegrationTest, OutUntouchedOnZeroStepLimit) {
    std::shared_ptr<expr> sentinel = id_term();
    std::shared_ptr<expr> out = sentinel;
    std::shared_ptr<expr> term = ap(ap(times_comb(), church(2)), church(2));
    EXPECT_FALSE(normalize_with_step_limit(out, term, 0));
    EXPECT_EQ(out, sentinel);
}

TEST_F(NormalizeIntegrationTest, MinimalStepLimitIISucceeds) {
    std::shared_ptr<expr> term = ap(id_term(), id_term());
    std::shared_ptr<expr> sentinel = k_term();
    std::shared_ptr<expr> out = sentinel;
    EXPECT_FALSE(normalize_with_step_limit(out, term, 0));
    EXPECT_EQ(out, sentinel);

    out = sentinel;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, OmegaEarlyStopLeavesOutUntouched) {
    std::shared_ptr<expr> sentinel = id_term();
    std::shared_ptr<expr> out = sentinel;
    uint64_t steps = 64;
    EXPECT_FALSE(normalize_with_step_limit(out, omega_term(), steps));
    EXPECT_EQ(out, sentinel);
}

// ---------------------------------------------------------------------------
// Priority: call-by-need memo (shared thunks)
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, SharedArgChargedOnceUnderW) {
    // W K (I I) → I. Arg (I I) is used twice syntactically; call-by-need forces
    // its WHNF once. Empirically needs 5 β-steps; an extra force of (I I) needs ≥6.
    std::shared_ptr<expr> term = ap(ap(w_term(), k_term()), ap(id_term(), id_term()));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, SelfAppSharedThunkKI) {
    // (λx. x x) (K I I) → I; shared thunk for (K I I).
    std::shared_ptr<expr> kii = ap(ap(k_term(), id_term()), id_term());
    std::shared_ptr<expr> term = ap(lm(ap(dv(0), dv(0))), kii);
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, DuplicateUseSameBindingNotDoubleForce) {
    // (λx. K (x I) (x K)) (I I) → I; x used twice, (I I) forced once for WHNF.
    std::shared_ptr<expr> body =
        ap(ap(k_term(), ap(dv(0), id_term())), ap(dv(0), k_term()));
    std::shared_ptr<expr> term = ap(lm(body), ap(id_term(), id_term()));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, WKKIdDiscardsOmega) {
    // W (K (K I)) Ω = (K (K I)) Ω Ω → (K I) Ω → I; both Ω uses discarded.
    // (Note: W (K I) Ω = I Ω = Ω, so that form must not be used here.)
    std::shared_ptr<expr> ki = ap(k_term(), id_term());
    std::shared_ptr<expr> term = ap(ap(w_term(), ap(k_term(), ki)), omega_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

// ---------------------------------------------------------------------------
// Priority: β-NF without η
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, ClosedEtaRedexPreserved) {
    // λy.λx. y x is β-nf; must not η-contract to λy. y.
    std::shared_ptr<expr> term = lm(lm(ap(dv(1), dv(0))));
    EXPECT_TRUE(exprs_eq(normalize(term), term));
}

TEST_F(NormalizeIntegrationTest, NestedEtaSpinePreserved) {
    std::shared_ptr<expr> term = lm(lm(lm(ap(ap(dv(2), dv(1)), dv(0)))));
    EXPECT_TRUE(exprs_eq(normalize(term), term));
}

TEST_F(NormalizeIntegrationTest, NeutralAppNotEtaContracted) {
    std::shared_ptr<expr> term = lm(lm(ap(dv(1), ap(dv(0), id_term()))));
    EXPECT_TRUE(exprs_eq(normalize(term), term));
}

// ---------------------------------------------------------------------------
// Priority: capture differentials
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, CapturedConstVsShadowedArg) {
    // Fun captures true; a wrong app-env leak would prefer false.
    std::shared_ptr<expr> true_c = true_comb();
    std::shared_ptr<expr> false_c = false_comb();
    std::shared_ptr<expr> inner = ap(lm(ap(dv(0), id_term())), lm(dv(1)));
    std::shared_ptr<expr> term = ap(ap(k_term(), ap(lm(inner), true_c)), false_c);
    EXPECT_TRUE(exprs_eq(normalize(term), true_c));
}

TEST_F(NormalizeIntegrationTest, NappArgSeesAppEnv) {
    // λa. λf. f (K a Ω) → λa. λf. f a  (arg of neutral must see a, discard Ω)
    std::shared_ptr<expr> arg = ap(ap(k_term(), dv(1)), omega_term());
    std::shared_ptr<expr> term = lm(lm(ap(dv(0), arg)));
    std::shared_ptr<expr> expected = lm(lm(ap(dv(0), dv(1))));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, expected));
}

TEST_F(NormalizeIntegrationTest, BetaExtendsFunEnvOnly) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(lm(lm(dv(1))), id_term()), k_term())), id_term()));
}

TEST_F(NormalizeIntegrationTest, BetaExtendsFunEnvOnlyWithOmega) {
    // λz. (λx.λy. x) z Ω → λz. z
    std::shared_ptr<expr> term = lm(ap(ap(lm(lm(dv(1))), dv(0)), omega_term()));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

// ---------------------------------------------------------------------------
// Priority: adversarial combinators / Church near-misses
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, SIIxIsSelfApp) {
    std::shared_ptr<expr> term =
        lm(ap(ap(ap(s_term(), id_term()), id_term()), dv(0)));
    std::shared_ptr<expr> expected = lm(ap(dv(0), dv(0)));
    EXPECT_TRUE(exprs_eq(normalize(term), expected));
}

TEST_F(NormalizeIntegrationTest, SKIAppliedIsIdentity) {
    // S (K I) I → I
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(s_term(), ap(k_term(), id_term())), id_term())),
        id_term()));
}

TEST_F(NormalizeIntegrationTest, PredSuccCancellationZero) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(pred_comb(), ap(succ_comb(), church(0)))), church(0)));
}

TEST_F(NormalizeIntegrationTest, PredSuccCancellationOne) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(pred_comb(), ap(succ_comb(), church(1)))), church(1)));
}

TEST_F(NormalizeIntegrationTest, PredSuccCancellationTwo) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(pred_comb(), ap(succ_comb(), church(2)))), church(2)));
}

TEST_F(NormalizeIntegrationTest, PredSuccCancellationThree) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(pred_comb(), ap(succ_comb(), church(3)))), church(3)));
}

TEST_F(NormalizeIntegrationTest, SuccPredCancellationOne) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(succ_comb(), ap(pred_comb(), church(1)))), church(1)));
}

TEST_F(NormalizeIntegrationTest, SuccPredCancellationTwo) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(succ_comb(), ap(pred_comb(), church(2)))), church(2)));
}

TEST_F(NormalizeIntegrationTest, SuccPredCancellationThree) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(succ_comb(), ap(pred_comb(), church(3)))), church(3)));
}

TEST_F(NormalizeIntegrationTest, IsZeroPredSuccZero) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(iszero_comb(),
                     ap(pred_comb(), ap(succ_comb(), church(0))))),
        true_comb()));
}

TEST_F(NormalizeIntegrationTest, DeepChurchRoundTrip) {
    // plus (times 3 2) (pred 5) = 6 + 4 = 10
    std::shared_ptr<expr> six = ap(ap(times_comb(), church(3)), church(2));
    std::shared_ptr<expr> four = ap(pred_comb(), church(5));
    std::shared_ptr<expr> term = ap(ap(plus_comb(), six), four);
    EXPECT_TRUE(exprs_eq(normalize(term), church(10)));
}

// ---------------------------------------------------------------------------
// Priority: deep index / recursion pressure
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, HighIndexTowerAlreadyNormal) {
    std::shared_ptr<expr> body = dv(19);
    for(uint32_t i = 0; i < 20; ++i)
        body = lm(body);
    EXPECT_TRUE(exprs_eq(normalize(body), body));
}

TEST_F(NormalizeIntegrationTest, HighIndexAfterBetaRematerialize) {
    // λ^20. (λ. 20) 0 → λ^20. 19
    std::shared_ptr<expr> inner = ap(lm(dv(20)), dv(0));
    std::shared_ptr<expr> term = inner;
    for(uint32_t i = 0; i < 20; ++i)
        term = lm(term);
    std::shared_ptr<expr> expected_body = dv(19);
    for(uint32_t i = 0; i < 20; ++i)
        expected_body = lm(expected_body);
    EXPECT_TRUE(exprs_eq(normalize(term), expected_body));
}

TEST_F(NormalizeIntegrationTest, DeepIdentitySpine) {
    std::shared_ptr<expr> term = id_term();
    for(int i = 0; i < 32; ++i)
        term = ap(id_term(), term);
    EXPECT_TRUE(exprs_eq(normalize(term), id_term()));
}

// ---------------------------------------------------------------------------
// Priority: fresh pools / isolation
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, SequentialNormalizeIndependent) {
    std::shared_ptr<expr> keep = ap(ap(k_term(), id_term()), omega_term());
    std::shared_ptr<expr> out1;
    run_normalize(out1, keep);
    EXPECT_TRUE(exprs_eq(out1, id_term()));

    std::shared_ptr<expr> sentinel = k_term();
    std::shared_ptr<expr> out2 = sentinel;
    EXPECT_FALSE(normalize_with_step_limit(out2, omega_term(), 16));
    EXPECT_EQ(out2, sentinel);
}

TEST_F(NormalizeIntegrationTest, IdempotentAfterHardTerm) {
    std::shared_ptr<expr> term =
        ap(pred_comb(), ap(ap(plus_comb(), church(2)), church(2)));
    std::shared_ptr<expr> once = normalize(term);
    std::shared_ptr<expr> twice = normalize(once);
    EXPECT_TRUE(exprs_eq(once, church(3)));
    EXPECT_TRUE(exprs_eq(twice, once));
}

// ---------------------------------------------------------------------------
// Second wave: Church arithmetic stress
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, ChurchPredZeroIsZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(pred_comb(), church(0))), church(0)));
}

TEST_F(NormalizeIntegrationTest, ChurchPredTwoIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(pred_comb(), church(2))), church(1)));
}

TEST_F(NormalizeIntegrationTest, ChurchPredThreeIsTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(pred_comb(), church(3))), church(2)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusZeroN) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(plus_comb(), church(0)), church(3))),
                         church(3)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusNZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(plus_comb(), church(3)), church(0))),
                         church(3)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusThreeTwoEqualsFive) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(plus_comb(), church(3)), church(2))),
                         church(5)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesZeroN) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(0)), church(3))),
                         church(0)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesThreeTwoEqualsSix) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(3)), church(2))),
                         church(6)));
}

TEST_F(NormalizeIntegrationTest, ChurchIsZeroTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(iszero_comb(), church(2))), false_comb()));
}

TEST_F(NormalizeIntegrationTest, ChurchIfIsZeroPredOne) {
    std::shared_ptr<expr> cond = ap(iszero_comb(), ap(pred_comb(), church(1)));
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(ap(if_comb(), cond), id_term()), k_term())),
        id_term()));
}

TEST_F(NormalizeIntegrationTest, ComposeThenApply) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(b_term(), id_term()), id_term())),
                         id_term()));
}

TEST_F(NormalizeIntegrationTest, SelfAppOfK) {
    // (λx. x x) K → K K = λy. K  (i.e. λ. K, since K = λλ.1)
    EXPECT_TRUE(exprs_eq(normalize(ap(lm(ap(dv(0), dv(0))), k_term())),
                         lm(k_term())));
}

TEST_F(NormalizeIntegrationTest, AlternateChurchTwoForms) {
    EXPECT_TRUE(exprs_eq(normalize(ap(succ_comb(), ap(succ_comb(), church(0)))),
                         normalize(church(2))));
}

// ---------------------------------------------------------------------------
// Second wave: hygiene / de Bruijn
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, TripleNestedBinderIndexShift) {
    // λa.λb.λc. (λx. 3) c → λa.λb.λc. a   (index 3 under four binders = a)
    std::shared_ptr<expr> term = lm(lm(lm(ap(lm(dv(3)), dv(0)))));
    std::shared_ptr<expr> expected = lm(lm(lm(dv(2))));
    EXPECT_TRUE(exprs_eq(normalize(term), expected));
}

TEST_F(NormalizeIntegrationTest, SharedThunkDistinctCells) {
    // Two independent bindings of (I I); forcing one must not break the other.
    // (λx. λy. K (x I) (y K)) (I I) (I I) → I
    std::shared_ptr<expr> ii = ap(id_term(), id_term());
    std::shared_ptr<expr> body =
        ap(ap(k_term(), ap(dv(1), id_term())), ap(dv(0), k_term()));
    std::shared_ptr<expr> term = ap(ap(lm(lm(body)), ii), ii);
    EXPECT_TRUE(exprs_eq(normalize(term), id_term()));
}

TEST_F(NormalizeIntegrationTest, QuoteInnerMostBinderIsZero) {
    EXPECT_TRUE(exprs_eq(normalize(lm(dv(0))), lm(dv(0))));
    EXPECT_TRUE(exprs_eq(normalize(lm(lm(dv(0)))), lm(lm(dv(0)))));
    EXPECT_TRUE(exprs_eq(normalize(lm(lm(dv(1)))), lm(lm(dv(1)))));
}

TEST_F(NormalizeIntegrationTest, CrossBinderReferenceUnderNeutral) {
    std::shared_ptr<expr> t10 = lm(lm(ap(dv(1), dv(0))));
    std::shared_ptr<expr> t01 = lm(lm(ap(dv(0), dv(1))));
    EXPECT_TRUE(exprs_eq(normalize(t10), t10));
    EXPECT_TRUE(exprs_eq(normalize(t01), t01));
}

TEST_F(NormalizeIntegrationTest, ReifyAfterBetaShiftsCorrectlyDeep) {
    // λ.λ. (λ. 2) 0 → λ.λ. 1
    std::shared_ptr<expr> term = lm(lm(ap(lm(dv(2)), dv(0))));
    std::shared_ptr<expr> expected = lm(lm(dv(1)));
    EXPECT_TRUE(exprs_eq(normalize(term), expected));
}

// ---------------------------------------------------------------------------
// Second wave: neutrals under λ
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, NestedNeutralSpine) {
    std::shared_ptr<expr> term = lm(ap(ap(dv(0), id_term()), id_term()));
    EXPECT_TRUE(exprs_eq(normalize(term), term));
}

TEST_F(NormalizeIntegrationTest, NeutralArgNormalizes) {
    std::shared_ptr<expr> term = lm(ap(dv(0), ap(id_term(), id_term())));
    std::shared_ptr<expr> expected = lm(ap(dv(0), id_term()));
    EXPECT_TRUE(exprs_eq(normalize(term), expected));
}

TEST_F(NormalizeIntegrationTest, NeutralFunWithExpensiveArg) {
    std::shared_ptr<expr> term = lm(ap(dv(0), ap(ap(k_term(), id_term()), id_term())));
    std::shared_ptr<expr> expected = lm(ap(dv(0), id_term()));
    EXPECT_TRUE(exprs_eq(normalize(term), expected));
}

// ---------------------------------------------------------------------------
// Second wave: idempotence / confluence proxies
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentIdentity) {
    std::shared_ptr<expr> once = normalize(id_term());
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentK) {
    std::shared_ptr<expr> once = normalize(k_term());
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentSKK) {
    std::shared_ptr<expr> term = ap(ap(s_term(), k_term()), k_term());
    std::shared_ptr<expr> once = normalize(term);
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentChurch3) {
    std::shared_ptr<expr> once = normalize(church(3));
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentPred2) {
    std::shared_ptr<expr> once = normalize(ap(pred_comb(), church(2)));
    EXPECT_TRUE(exprs_eq(normalize(once), once));
    EXPECT_TRUE(exprs_eq(once, church(1)));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentEtaRedex) {
    std::shared_ptr<expr> term = lm(lm(ap(dv(1), dv(0))));
    std::shared_ptr<expr> once = normalize(term);
    EXPECT_TRUE(exprs_eq(normalize(once), once));
    EXPECT_TRUE(exprs_eq(once, term));
}

TEST_F(NormalizeIntegrationTest, NormalizeIdempotentNeutral) {
    std::shared_ptr<expr> term = lm(ap(ap(dv(0), id_term()), id_term()));
    std::shared_ptr<expr> once = normalize(term);
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

TEST_F(NormalizeIntegrationTest, WCombWithCostlySharedArg) {
    // W (λa.λb. a) (I I) → I
    std::shared_ptr<expr> fst = lm(lm(dv(1)));
    std::shared_ptr<expr> term = ap(ap(w_term(), fst), ap(id_term(), id_term()));
    EXPECT_TRUE(exprs_eq(normalize(term), id_term()));
}

TEST_F(NormalizeIntegrationTest, SelfAppMemoDoesNotCorrupt) {
    std::shared_ptr<expr> kii = ap(ap(k_term(), id_term()), id_term());
    EXPECT_TRUE(exprs_eq(normalize(ap(lm(ap(dv(0), dv(0))), kii)), id_term()));
}

// ---------------------------------------------------------------------------
// Normal-order agreement / multi-redex
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, NormalOrderKDiscardsInnerOmega) {
    // ((λx.λy. y) Ω) I → I
    std::shared_ptr<expr> term = ap(ap(lm(lm(dv(0))), omega_term()), id_term());
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, NormalOrderComposeDiscards) {
    // (λx. I) (Ω Ω) → I
    std::shared_ptr<expr> junk = ap(omega_term(), omega_term());
    std::shared_ptr<expr> term = ap(lm(id_term()), junk);
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, NormalOrderUnderBinder) {
    // λz. (λx. z) Ω → λz. z
    std::shared_ptr<expr> term = lm(ap(lm(dv(1)), omega_term()));
    std::shared_ptr<expr> out;
    run_normalize(out, term);
    EXPECT_TRUE(exprs_eq(out, id_term()));
}

TEST_F(NormalizeIntegrationTest, NormalOrderLeftNestedRedexes) {
    // ((λx.λy. x) I) ((λa.a) K) → I
    std::shared_ptr<expr> left = ap(lm(lm(dv(1))), id_term());
    std::shared_ptr<expr> right = ap(lm(dv(0)), k_term());
    EXPECT_TRUE(exprs_eq(normalize(ap(left, right)), id_term()));
}

TEST_F(NormalizeIntegrationTest, NormalOrderAgreesIndependentConstruction) {
    std::shared_ptr<expr> via_plus = normalize(ap(ap(plus_comb(), church(2)), church(3)));
    std::shared_ptr<expr> via_succ = normalize(
        ap(succ_comb(), ap(succ_comb(), ap(succ_comb(), church(2)))));
    EXPECT_TRUE(exprs_eq(via_plus, church(5)));
    EXPECT_TRUE(exprs_eq(via_plus, via_succ));
}

// ---------------------------------------------------------------------------
// Recursion via Y
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, YOmegaLikeDiverges) {
    std::shared_ptr<expr> term = ap(y_comb(), id_term());
    std::shared_ptr<expr> sentinel = k_term();
    std::shared_ptr<expr> out = sentinel;
    uint64_t budget = 64;
    EXPECT_FALSE(normalize_with_step_limit(out, term, budget));
    EXPECT_EQ(out, sentinel);
}

TEST_F(NormalizeIntegrationTest, YFactZeroIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_comb(), church(0))), church(1)));
}

TEST_F(NormalizeIntegrationTest, YFactOneIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_comb(), church(1))), church(1)));
}

TEST_F(NormalizeIntegrationTest, YFactThreeIsSix) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_comb(), church(3))), church(6)));
}

TEST_F(NormalizeIntegrationTest, YFactFiveIsOneTwenty) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_comb(), church(5))), church(120)));
}

TEST_F(NormalizeIntegrationTest, YFactNineIs362880) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_comb(), church(9))), church(362880)));
}

TEST_F(NormalizeIntegrationTest, IterFactZeroIsOne) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_iter_comb(), church(0))), church(1)));
}

TEST_F(NormalizeIntegrationTest, IterFactThreeIsSix) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_iter_comb(), church(3))), church(6)));
}

TEST_F(NormalizeIntegrationTest, IterFactFiveIsOneTwenty) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_iter_comb(), church(5))), church(120)));
}

TEST_F(NormalizeIntegrationTest, IterFactNineIs362880) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_iter_comb(), church(9))), church(362880)));
}

TEST_F(NormalizeIntegrationTest, BinSuccZeroIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(bin_succ_comb(), bin(0))), normalize(bin(1))));
}

TEST_F(NormalizeIntegrationTest, BinSuccThreeIsFour) {
    EXPECT_TRUE(exprs_eq(normalize(ap(bin_succ_comb(), bin(3))), normalize(bin(4))));
}

TEST_F(NormalizeIntegrationTest, BinAddTwoThreeIsFive) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(bin_add_comb(), bin(2)), bin(3))), normalize(bin(5))));
}

TEST_F(NormalizeIntegrationTest, BinTimesThreeFourIsTwelve) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(bin_times_comb(), bin(3)), bin(4))), normalize(bin(12))));
}

TEST_F(NormalizeIntegrationTest, BinFactZeroIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_bin_iter_comb(), church(0))),
                         normalize(bin(1))));
}

TEST_F(NormalizeIntegrationTest, BinFactFiveIsOneTwenty) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_bin_iter_comb(), church(5))),
                         normalize(bin(120))));
}

TEST_F(NormalizeIntegrationTest, BinFactNineIs362880) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_bin_iter_comb(), church(9))),
                         normalize(bin(362880))));
}

// Unshared n (two thunks): stress test. 8! = 40320; 9! = 362880.
TEST_F(NormalizeIntegrationTest, YFactEightNoshareStress) {
    EXPECT_TRUE(
        exprs_eq(normalize(ap(fact_noshare_comb(), church(8))), church(40320)));
}

TEST_F(NormalizeIntegrationTest, YFactNineNoshareStress) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fact_noshare_comb(), church(9))),
                         church(362880)));
}

TEST_F(NormalizeIntegrationTest, YFibZeroIsZero) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fib_comb(), church(0))), church(0)));
}

TEST_F(NormalizeIntegrationTest, YFibOneIsOne) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fib_comb(), church(1))), church(1)));
}

TEST_F(NormalizeIntegrationTest, YFibSixIsEight) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fib_comb(), church(6))), church(8)));
}

TEST_F(NormalizeIntegrationTest, YFibSevenIsThirteen) {
    EXPECT_TRUE(exprs_eq(normalize(ap(fib_comb(), church(7))), church(13)));
}

// ---------------------------------------------------------------------------
// Calculation stress (Church)
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, ChurchExpTwoThreeIsEight) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(exp_comb(), church(2)), church(3))),
                         church(8)));
}

TEST_F(NormalizeIntegrationTest, ChurchExpThreeThreeIsTwentySeven) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(exp_comb(), church(3)), church(3))),
                         church(27)));
}

TEST_F(NormalizeIntegrationTest, ChurchExpTwoFiveIsThirtyTwo) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(exp_comb(), church(2)), church(5))),
                         church(32)));
}

TEST_F(NormalizeIntegrationTest, ChurchTimesFiveFiveIsTwentyFive) {
    EXPECT_TRUE(exprs_eq(normalize(ap(ap(times_comb(), church(5)), church(5))),
                         church(25)));
}

TEST_F(NormalizeIntegrationTest, ChurchPlusTenTenIsTwenty) {
    EXPECT_TRUE(exprs_eq(
        normalize(ap(ap(plus_comb(), church(10)), church(10))), church(20)));
}

TEST_F(NormalizeIntegrationTest, ChurchMixedStress) {
    // plus (times 4 5) (exp 2 3) = 20 + 8 = 28
    std::shared_ptr<expr> term =
        ap(ap(plus_comb(), ap(ap(times_comb(), church(4)), church(5))),
           ap(ap(exp_comb(), church(2)), church(3)));
    EXPECT_TRUE(exprs_eq(normalize(term), church(28)));
}

TEST_F(NormalizeIntegrationTest, ChurchPredChain) {
    std::shared_ptr<expr> term = church(8);
    for(int i = 0; i < 5; ++i)
        term = ap(pred_comb(), term);
    EXPECT_TRUE(exprs_eq(normalize(term), church(3)));
}

TEST_F(NormalizeIntegrationTest, ChurchIdempotentLarge) {
    std::shared_ptr<expr> once =
        normalize(ap(ap(times_comb(), church(4)), church(5)));
    EXPECT_TRUE(exprs_eq(once, church(20)));
    EXPECT_TRUE(exprs_eq(normalize(once), once));
}

// ---------------------------------------------------------------------------
// Recursion + arithmetic confluence
// ---------------------------------------------------------------------------

TEST_F(NormalizeIntegrationTest, YFactThreeEqualsTimesThreeTwo) {
    std::shared_ptr<expr> via_fact = normalize(ap(fact_comb(), church(3)));
    std::shared_ptr<expr> via_times =
        normalize(ap(ap(times_comb(), church(3)), church(2)));
    EXPECT_TRUE(exprs_eq(via_fact, church(6)));
    EXPECT_TRUE(exprs_eq(via_fact, via_times));
}

TEST_F(NormalizeIntegrationTest, YFibPlusTable) {
    // plus (fib 5) (fib 6) = 5 + 8 = 13
    std::shared_ptr<expr> term = ap(ap(plus_comb(), ap(fib_comb(), church(5))),
                          ap(fib_comb(), church(6)));
    EXPECT_TRUE(exprs_eq(normalize(term), church(13)));
}
