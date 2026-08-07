#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>

template <typename IEval, typename IReify> struct normalizer {
    normalizer(IEval& eval, IReify& reify);
    bool normalize(const expr*& out, const expr* term,
                   uint64_t& reductions_left);

  private:
    IEval& eval_;
    IReify& reify_;
};

template <typename IEval, typename IReify>
normalizer<IEval, IReify>::normalizer(IEval& eval, IReify& reify)
    : eval_(eval), reify_(reify) {
}

template <typename IEval, typename IReify>
bool normalizer<IEval, IReify>::normalize(const expr*& out, const expr* term,
                                          uint64_t& reductions_left) {
    const val* whnf;
    if(!eval_.eval(whnf, term, nullptr, reductions_left))
        return false;
    return reify_.reify(out, whnf, 0, reductions_left);
}

#endif
