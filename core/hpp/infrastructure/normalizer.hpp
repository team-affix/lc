#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"
#include "value_objects/val.hpp"

template <typename IMakeClo> struct normalizer {
    normalizer(IMakeClo& make_clo);
    funcall normalize(const expr*& out, const expr* term);

  private:
    IMakeClo& make_clo_;
};

template <typename IMakeClo>
normalizer<IMakeClo>::normalizer(IMakeClo& make_clo) : make_clo_(make_clo) {
}

template <typename IMakeClo>
funcall normalizer<IMakeClo>::normalize(const expr*& out, const expr* term) {
    const val* seed = make_clo_.make_clo(term, nullptr);
    return reify_val_funcall{out, seed, 0};
}

#endif
