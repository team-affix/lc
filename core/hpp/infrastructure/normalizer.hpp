#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include "value_objects/expr.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_stage.hpp"
#include "value_objects/val.hpp"

template <typename IMakeClo> struct normalizer {
    normalizer(IMakeClo& make_clo);
    frame normalize(const expr*& out, const expr* term);

  private:
    IMakeClo& make_clo_;
};

template <typename IMakeClo>
normalizer<IMakeClo>::normalizer(IMakeClo& make_clo) : make_clo_(make_clo) {
}

template <typename IMakeClo>
frame normalizer<IMakeClo>::normalize(const expr*& out, const expr* term) {
    const val* seed = make_clo_.make_clo(term, nullptr);
    return reify_val_frame{out, seed, 0, reify_val_stage::need_whnf};
}

#endif
