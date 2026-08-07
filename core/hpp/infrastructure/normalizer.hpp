#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"
#include <cstdint>

template <typename IMakeClo, typename IReify> struct normalizer {
    normalizer(IMakeClo& make_clo, IReify& reify);
    bool normalize(const expr*& out, const expr* term,
                   uint64_t& reductions_left);

  private:
    IMakeClo& make_clo_;
    IReify& reify_;
};

template <typename IMakeClo, typename IReify>
normalizer<IMakeClo, IReify>::normalizer(IMakeClo& make_clo, IReify& reify)
    : make_clo_(make_clo), reify_(reify) {
}

template <typename IMakeClo, typename IReify>
bool normalizer<IMakeClo, IReify>::normalize(const expr*& out,
                                             const expr* term,
                                             uint64_t& reductions_left) {
    const val* seed = make_clo_.make_clo(term, nullptr);
    return reify_.reify(out, seed, 0, reductions_left);
}

#endif
