#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "debug_assert.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/funcall.hpp"
#include <cstddef>
#include <deque>
#include <optional>
#include <utility>
#include <variant>

template <typename Continuation, typename IProcess, typename IInitContinuation>
struct interpreter {
    interpreter(IProcess& process, IInitContinuation& init_continuation,
                funcall initial);
    void step();
    bool done() const;
    std::size_t space_usage() const;

  private:
    std::deque<Continuation> stack_;
    IProcess& process_;
    IInitContinuation& init_continuation_;
};

template <typename Continuation, typename IProcess, typename IInitContinuation>
interpreter<Continuation, IProcess, IInitContinuation>::interpreter(
    IProcess& process, IInitContinuation& init_continuation, funcall initial)
    : stack_(), process_(process), init_continuation_(init_continuation) {
    stack_.push_back(std::visit(
        [this](auto& fc) { return init_continuation_.init_continuation(fc); },
        initial));
}

template <typename Continuation, typename IProcess, typename IInitContinuation>
void interpreter<Continuation, IProcess, IInitContinuation>::step() {
    DEBUG_ASSERT(!stack_.empty());
    std::optional<funcall> child = std::visit(
        [this](auto& c) -> std::optional<funcall> {
            return std::visit(
                [this, &c](auto& s) -> std::optional<funcall> {
                    auto result = process_.process(c.frame, s);
                    if(!result.has_value())
                        return std::nullopt;
                    c.stage = std::move(result->first);
                    return std::move(result->second);
                },
                c.stage);
        },
        stack_.back());
    if(!child.has_value())
        stack_.pop_back();
    else
        stack_.push_back(std::visit(
            [this](auto& fc) {
                return init_continuation_.init_continuation(fc);
            },
            *child));
}

template <typename Continuation, typename IProcess, typename IInitContinuation>
bool interpreter<Continuation, IProcess, IInitContinuation>::done() const {
    return stack_.empty();
}

template <typename Continuation, typename IProcess, typename IInitContinuation>
std::size_t
interpreter<Continuation, IProcess, IInitContinuation>::space_usage() const {
    return stack_.size() * sizeof(Continuation);
}

#endif
