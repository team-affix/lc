#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <variant>
#include "infrastructure/interpreter.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reduce_app_funcall.hpp"
#include "value_objects/reduce_var_funcall.hpp"
#include "value_objects/reduce_whnf_funcall.hpp"
#include "value_objects/reify_clo_funcall.hpp"
#include "value_objects/reify_napp_funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"
#include "value_objects/val.hpp"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

namespace {

struct test_frame {};
struct test_stage {};
struct test_continuation_alt {
    test_frame frame;
    std::variant<test_stage> stage;
};
using test_continuation = std::variant<test_continuation_alt>;

using process_result_t =
    std::optional<std::pair<std::variant<test_stage>, std::optional<funcall>>>;

struct MockProcess {
    MOCK_METHOD(process_result_t, process, (test_frame&, test_stage&), ());
};

struct MockInitContinuation {
    MOCK_METHOD(test_continuation, init_reify_val, (reify_val_funcall&), ());

    test_continuation init_continuation(reify_val_funcall& fc) {
        return init_reify_val(fc);
    }
    test_continuation init_continuation(reduce_whnf_funcall&) {
        throw std::logic_error("unexpected reduce_whnf_funcall");
    }
    test_continuation init_continuation(reduce_app_funcall&) {
        throw std::logic_error("unexpected reduce_app_funcall");
    }
    test_continuation init_continuation(reduce_var_funcall&) {
        throw std::logic_error("unexpected reduce_var_funcall");
    }
    test_continuation init_continuation(reify_clo_funcall&) {
        throw std::logic_error("unexpected reify_clo_funcall");
    }
    test_continuation init_continuation(reify_napp_funcall&) {
        throw std::logic_error("unexpected reify_napp_funcall");
    }
};

} // namespace

struct InterpreterTest : public ::testing::Test {
    NiceMock<MockProcess> process;
    NiceMock<MockInitContinuation> init_continuation;
    std::shared_ptr<expr> out;
    std::shared_ptr<val> seed;

    InterpreterTest()
        : process()
        , init_continuation()
        , out()
        , seed(std::make_shared<val>(val{val::fvar{0}})) {
    }

    test_continuation make_cont() {
        return test_continuation{test_continuation_alt{
            test_frame{}, std::variant<test_stage>{test_stage{}}}};
    }
};

TEST_F(InterpreterTest, StepOnEmptyStackThrows) {
    EXPECT_CALL(init_continuation, init_reify_val(_))
        .WillOnce(Return(make_cont()));
    EXPECT_CALL(process, process(_, _)).WillOnce(Return(std::nullopt));

    interpreter<test_continuation, MockProcess, MockInitContinuation> interp{
        process, init_continuation, reify_val_funcall{out, seed, 0}};
    EXPECT_FALSE(interp.done());
    interp.step();
    EXPECT_TRUE(interp.done());
    EXPECT_THROW(interp.step(), std::logic_error);
}
