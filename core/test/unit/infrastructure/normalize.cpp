#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <limits>
#include "exprs_eq.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/val_pool.hpp"
#include "nbe_fixture.hpp"

using ::testing::DoAll;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SetArgReferee;
using ::testing::_;

struct MockMakeClo {
    MOCK_METHOD(const val*, make_clo, (const expr*, env*), ());
};

struct MockReify {
    MOCK_METHOD(bool, reify, (const expr*&, const val*&, uint32_t, uint64_t&),
                ());
};

using test_normalizer_t = normalizer<MockMakeClo, MockReify>;

struct NormalizerMockTest : public ::testing::Test {
    NiceMock<MockMakeClo> make_clo;
    NiceMock<MockReify> reify;
    test_normalizer_t norm{make_clo, reify};
    expr_pool pool;
    val_pool vals;
    uint64_t budget{std::numeric_limits<uint64_t>::max()};
};

TEST_F(NormalizerMockTest, NormalizeSeedsCloAndReifiesAtDepthZero) {
    const expr* term = pool.make_abs(pool.make_var(0));
    const val* seed = vals.make_clo(term, nullptr);
    const expr* result = term;
    {
        ::testing::InSequence seq;
        EXPECT_CALL(make_clo, make_clo(term, nullptr)).WillOnce(Return(seed));
        EXPECT_CALL(reify, reify(_, _, 0, _))
            .WillOnce(DoAll(SetArgReferee<0>(result), Return(true)));
    }
    const expr* got;
    ASSERT_TRUE(norm.normalize(got, term, budget));
    EXPECT_EQ(got, result);
}

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
