#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>
#include <optional>

template <typename IEval, typename IReify>
struct normalizer {
    normalizer(IEval& eval, IReify& reify);
    std::optional<const expr*> normalize(const expr* term,
                                         uint64_t& reductions_left);

private:
    IEval& eval_;
    IReify& reify_;
};

template <typename IEval, typename IReify>
normalizer<IEval, IReify>::normalizer(IEval& eval, IReify& reify)
    : eval_(eval), reify_(reify) {}

template <typename IEval, typename IReify>
std::optional<const expr*>
normalizer<IEval, IReify>::normalize(const expr* term,
                                     uint64_t& reductions_left) {
    std::optional<const val*> whnf = eval_.eval(term, nullptr, reductions_left);
    if (!whnf.has_value())
        return std::nullopt;
    return reify_.reify(*whnf, 0, reductions_left);
}

#endif
