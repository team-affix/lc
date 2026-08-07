#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <limits>
#include <optional>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/val_pool.hpp"
#include "nbe_fixture.hpp"

using ::testing::NiceMock;
using ::testing::Return;
using ::testing::_;

struct MockEval {
    MOCK_METHOD((std::optional<const val*>), eval,
                (const expr*, const env*, uint64_t&), ());
};

struct MockReify {
    MOCK_METHOD((std::optional<const expr*>), reify,
                (const val*, uint32_t, uint64_t&), ());
};

using test_normalizer_t = normalizer<MockEval, MockReify>;

struct NormalizerMockTest : public ::testing::Test {
    NiceMock<MockEval> eval;
    NiceMock<MockReify> reify;
    test_normalizer_t norm{eval, reify};
    expr_pool pool;
    val_pool vals;
    uint64_t budget{std::numeric_limits<uint64_t>::max()};
};

TEST_F(NormalizerMockTest, NormalizeEvalThenReify) {
    const expr* term = pool.make_abs(pool.make_var(0));
    const val* whnf = vals.make_clo(pool.make_var(0), nullptr);
    const expr* result = pool.make_abs(pool.make_var(0));
    {
        ::testing::InSequence seq;
        EXPECT_CALL(eval, eval(term, nullptr, _))
            .WillOnce(Return(std::optional<const val*>{whnf}));
        EXPECT_CALL(reify, reify(whnf, 0, _))
            .WillOnce(Return(std::optional<const expr*>{result}));
    }
    std::optional<const expr*> got = norm.normalize(term, budget);
    ASSERT_TRUE(got.has_value());
    EXPECT_EQ(*got, result);
}

struct NormalizerTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizerTest, NormalizeAbsVar) {
    EXPECT_TRUE(exprs_eq(normalize(lm(dv(0))), id_term()));
}

TEST_F(NormalizerTest, NormalizeIdentityOnIdentity) {
    EXPECT_TRUE(exprs_eq(normalize(ap(id_term(), id_term())), id_term()));
}

TEST_F(NormalizerTest, NormalizeExhaustsBetaBudget) {
    const expr* term = ap(id_term(), id_term());
    EXPECT_EQ(normalize_with_budget(term, 0), std::nullopt);
    EXPECT_TRUE(exprs_eq(normalize_with_budget(term, 1).value(), id_term()));
}
