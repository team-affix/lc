#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "value_objects/continuation.hpp"
#include "value_objects/funcall.hpp"
#include <deque>
#include <optional>
#include <utility>
#include <variant>

template <typename IContinuation, typename IProcessor> struct interpreter {
    interpreter(IProcessor& processor, funcall initial);
    bool step();
    bool done();

  private:
    std::deque<IContinuation> stack_;
    IProcessor& processor_;
};

template <typename IContinuation, typename IProcessor>
interpreter<IContinuation, IProcessor>::interpreter(IProcessor& processor,
                                                    funcall initial)
    : stack_(), processor_(processor) {
    stack_.push_back(processor_.init_continuation(std::move(initial)));
}

template <typename IContinuation, typename IProcessor>
bool interpreter<IContinuation, IProcessor>::step() {
    if(stack_.empty())
        return false;
    std::optional<funcall> child;
    std::visit(
        [this, &child](auto& c) {
            std::visit(
                [this, &c, &child](auto& s) {
                    auto result = processor_.process(c.frame, s);
                    c.stage = std::move(result.first);
                    child = std::nullopt;
                    if(result.second.has_value())
                        child.emplace(std::move(*result.second));
                },
                c.stage);
        },
        stack_.back());
    if(!child.has_value())
        stack_.pop_back();
    else
        stack_.push_back(processor_.init_continuation(std::move(*child)));
    return true;
}

template <typename IContinuation, typename IProcessor>
bool interpreter<IContinuation, IProcessor>::done() {
    return stack_.empty();
}

#endif
