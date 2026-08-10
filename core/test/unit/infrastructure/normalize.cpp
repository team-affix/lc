#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/val_pool.hpp"
#include "nbe_fixture.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

using test_normalizer_t = normalizer<MockMakeClo>;

struct NormalizerMockTest : public ::testing::Test {
    NiceMock<MockMakeClo> make_clo;
    test_normalizer_t norm{make_clo};
    expr_pool pool;
    val_pool vals;
};

TEST_F(NormalizerMockTest, NormalizeSeedsCloAndReturnsReifyValFunCall) {
    const expr* term = pool.make_abs(pool.make_var(0));
    const val* seed = vals.make_clo(term, nullptr);
    EXPECT_CALL(make_clo, make_clo(term, nullptr)).WillOnce(Return(seed));
    const expr* got = nullptr;
    funcall root = norm.normalize(got, term);
    reify_val_funcall* f = std::get_if<reify_val_funcall>(&root);
    ASSERT_NE(f, nullptr);
    EXPECT_EQ(f->v, seed);
    EXPECT_EQ(f->depth, 0u);
}

struct NormalizeTest : public ::testing::Test, public nbe_fixture {};

TEST_F(NormalizeTest, NormalizeAbsVar) {
    EXPECT_TRUE(exprs_eq(normalize(lm(dv(0))), id_term()));
}

TEST_F(NormalizeTest, NormalizeIdentityOnIdentity) {
    EXPECT_TRUE(exprs_eq(normalize(ap(id_term(), id_term())), id_term()));
}

TEST_F(NormalizeTest, EarlyStopLeavesWorkIncomplete) {
    const expr* term = ap(id_term(), id_term());
    const expr* out = nullptr;
    EXPECT_FALSE(normalize_with_step_limit(out, term, 0));
}
