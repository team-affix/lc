#ifndef NORMALIZER_HPP
#define NORMALIZER_HPP

#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"
#include "value_objects/val.hpp"

template <typename IMakeClo> struct normalizer {
    normalizer(IMakeClo& make_clo);
    funcall normalize(std::shared_ptr<expr>& out, std::shared_ptr<expr> term);

  private:
    IMakeClo& make_clo_;
};

template <typename IMakeClo>
normalizer<IMakeClo>::normalizer(IMakeClo& make_clo) : make_clo_(make_clo) {
}

template <typename IMakeClo>
funcall normalizer<IMakeClo>::normalize(std::shared_ptr<expr>& out,
                                        std::shared_ptr<expr> term) {
    std::shared_ptr<val> seed = make_clo_.make_clo(std::move(term), {});
    return reify_val_funcall{out, std::move(seed), 0};
}

#endif
