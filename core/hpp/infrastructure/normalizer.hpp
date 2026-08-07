#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

template <typename IEval, typename IReify> struct normalizer {
    normalizer(IEval& eval, IReify& reify);
    const expr* normalize(const expr* term);

  private:
    IEval& eval_;
    IReify& reify_;
};

template <typename IEval, typename IReify>
normalizer<IEval, IReify>::normalizer(IEval& eval, IReify& reify)
    : eval_(eval), reify_(reify) {
}

template <typename IEval, typename IReify>
const expr* normalizer<IEval, IReify>::normalize(const expr* term) {
    const val* whnf = eval_.eval(term, nullptr);
    return reify_.reify(whnf, 0);
}

#endif
