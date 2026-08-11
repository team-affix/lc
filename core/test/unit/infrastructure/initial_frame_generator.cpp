#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <variant>
#include "exprs_eq.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/initial_frame_generator.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/val_factory.hpp"
#include "nbe_fixture.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"

using ::testing::NiceMock;
using ::testing::Return;

struct MockMakeClo {
    MOCK_METHOD(std::shared_ptr<val>, make_clo,
                (std::shared_ptr<expr>, std::shared_ptr<env>), ());
};

using test_initial_frame_generator_t = initial_frame_generator<MockMakeClo>;

struct InitialFrameGeneratorMockTest : public ::testing::Test {
    rc_pool<expr> expr_nodes;
    rc_pool<val> val_nodes;
    expr_factory<rc_pool<expr>> pool;
    val_factory<rc_pool<val>> vals;
    NiceMock<MockMakeClo> make_clo;
    test_initial_frame_generator_t initial_frame_gen{make_clo};

    InitialFrameGeneratorMockTest()
        : expr_nodes(), val_nodes(), pool(expr_nodes), vals(val_nodes) {
    }
};

TEST_F(InitialFrameGeneratorMockTest,
       GenerateInitialFrameSeedsCloAndReturnsReifyValFunCall) {
    auto term = pool.make_abs(pool.make_var(0));
    auto seed = vals.make_clo(term, {});
    EXPECT_CALL(make_clo, make_clo(term, std::shared_ptr<env>{}))
        .WillOnce(Return(seed));
    std::shared_ptr<expr> got;
    funcall root = initial_frame_gen.generate_initial_frame(got, term);
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
    auto term = ap(id_term(), id_term());
    std::shared_ptr<expr> sentinel = k_term();
    std::shared_ptr<expr> out = sentinel;
    EXPECT_FALSE(normalize_with_step_limit(out, term, 0));
    EXPECT_EQ(out, sentinel);
}
