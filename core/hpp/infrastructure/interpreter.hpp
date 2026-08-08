#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <deque>

template <typename IFrame, typename IProcessor> struct interpreter {
    interpreter(IProcessor& processor, IFrame initial);
    bool step();
    bool done();

  private:
    std::deque<IFrame> stack_;
    IProcessor& processor_;
};

template <typename IFrame, typename IProcessor>
interpreter<IFrame, IProcessor>::interpreter(IProcessor& processor,
                                             IFrame initial)
    : stack_(), processor_(processor) {
    stack_.push_back(std::move(initial));
}

template <typename IFrame, typename IProcessor>
bool interpreter<IFrame, IProcessor>::step() {
    if(stack_.empty())
        return false;
    if(auto child = processor_.process(stack_.back()))
        stack_.push_back(std::move(*child));
    else
        stack_.pop_back();
    return true;
}

template <typename IFrame, typename IProcessor>
bool interpreter<IFrame, IProcessor>::done() {
    return stack_.empty();
}

#endif
