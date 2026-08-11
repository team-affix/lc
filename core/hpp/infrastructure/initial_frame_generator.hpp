#ifndef INITIAL_FRAME_GENERATOR_HPP
#define INITIAL_FRAME_GENERATOR_HPP

#include <memory>
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reify_val_funcall.hpp"
#include "value_objects/val.hpp"

template <typename IMakeClo> struct initial_frame_generator {
    initial_frame_generator(IMakeClo& make_clo);
    funcall generate_initial_frame(std::shared_ptr<expr>& out,
                                   std::shared_ptr<expr> term);

  private:
    IMakeClo& make_clo_;
};

template <typename IMakeClo>
initial_frame_generator<IMakeClo>::initial_frame_generator(IMakeClo& make_clo)
    : make_clo_(make_clo) {
}

template <typename IMakeClo>
funcall initial_frame_generator<IMakeClo>::generate_initial_frame(
    std::shared_ptr<expr>& out, std::shared_ptr<expr> term) {
    std::shared_ptr<val> seed = make_clo_.make_clo(std::move(term), {});
    return reify_val_funcall{out, std::move(seed), 0};
}

#endif
