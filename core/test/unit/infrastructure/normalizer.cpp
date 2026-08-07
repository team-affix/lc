#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/val_pool.hpp"
#include "nbe_fixture.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockEval {
    MOCK_METHOD(const val*, eval, (const expr*, const env*), ());
};

struct MockReify {
    MOCK_METHOD(const expr*, reify, (const val*, uint32_t), ());
};

using test_normalizer_t = normalizer<MockEval, MockReify>;

struct NormalizerMockTest : public ::testing::Test {
    NiceMock<MockEval> eval;
    NiceMock<MockReify> reify;
    test_normalizer_t norm{eval, reify};
    expr_pool pool;
    val_pool vals;
};

TEST_F(NormalizerMockTest, NormalizeEvalThenReify) {
    const expr* term = pool.make_abs(pool.make_var(0));
    const val* whnf = vals.make_clo(pool.make_var(0), nullptr);
    const expr* result = pool.make_abs(pool.make_var(0));
    {
        ::testing::InSequence seq;
        EXPECT_CALL(eval, eval(term, nullptr)).WillOnce(Return(whnf));
        EXPECT_CALL(reify, reify(whnf, 0)).WillOnce(Return(result));
    }
    EXPECT_EQ(norm.normalize(term), result);
}

struct NormalizerTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizerTest, NormalizeAbsVar) {
    EXPECT_EQ(normalize(lm(dv(0))), id_term());
}

TEST_F(NormalizerTest, NormalizeIdentityOnIdentity) {
    EXPECT_EQ(normalize(ap(id_term(), id_term())), id_term());
}
