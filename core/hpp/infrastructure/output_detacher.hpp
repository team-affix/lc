#ifndef OUTPUT_DETACHER_HPP
#define OUTPUT_DETACHER_HPP

#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include "value_objects/detach_expr_after_arg_stage.hpp"
#include "value_objects/detach_expr_after_body_stage.hpp"
#include "value_objects/detach_expr_after_fun_stage.hpp"
#include "value_objects/detach_expr_continuation.hpp"
#include "value_objects/detach_expr_frame.hpp"
#include "value_objects/detach_expr_funcall.hpp"
#include "value_objects/detach_expr_stage.hpp"
#include "value_objects/detach_expr_start_stage.hpp"
#include "value_objects/expr.hpp"

template <typename IAllocExpr> struct output_detacher {
    output_detacher(IAllocExpr& alloc_expr);
    detach_expr_continuation init_continuation(detach_expr_funcall fc);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_start_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_body_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_fun_stage);
    std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
    process(detach_expr_frame& f, detach_expr_after_arg_stage);

  private:
    IAllocExpr& alloc_expr_;
};

template <typename IAllocExpr>
output_detacher<IAllocExpr>::output_detacher(IAllocExpr& alloc_expr)
    : alloc_expr_(alloc_expr) {
}

template <typename IAllocExpr>
detach_expr_continuation
output_detacher<IAllocExpr>::init_continuation(detach_expr_funcall fc) {
    return detach_expr_continuation{
        detach_expr_frame{fc.out, std::move(fc.src), std::shared_ptr<expr>{},
                          std::shared_ptr<expr>{}, std::shared_ptr<expr>{}},
        detach_expr_start_stage{}};
}

template <typename IAllocExpr>
std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher<IAllocExpr>::process(detach_expr_frame& f,
                                     detach_expr_start_stage) {
    if(const expr::var* v = std::get_if<expr::var>(&f.src->content)) {
        f.out = alloc_expr_.alloc(expr{expr::var{v->index}});
        return std::nullopt;
    }
    if(const expr::abs* a = std::get_if<expr::abs>(&f.src->content)) {
        return std::pair<detach_expr_stage, detach_expr_funcall>{
            detach_expr_after_body_stage{},
            detach_expr_funcall{f.body, a->body}};
    }
    const expr::app& ap = std::get<expr::app>(f.src->content);
    return std::pair<detach_expr_stage, detach_expr_funcall>{
        detach_expr_after_fun_stage{}, detach_expr_funcall{f.fun, ap.fun}};
}

template <typename IAllocExpr>
std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher<IAllocExpr>::process(detach_expr_frame& f,
                                     detach_expr_after_body_stage) {
    f.out = alloc_expr_.alloc(expr{expr::abs{f.body}});
    return std::nullopt;
}

template <typename IAllocExpr>
std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher<IAllocExpr>::process(detach_expr_frame& f,
                                     detach_expr_after_fun_stage) {
    const expr::app& ap = std::get<expr::app>(f.src->content);
    return std::pair<detach_expr_stage, detach_expr_funcall>{
        detach_expr_after_arg_stage{}, detach_expr_funcall{f.arg, ap.arg}};
}

template <typename IAllocExpr>
std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher<IAllocExpr>::process(detach_expr_frame& f,
                                     detach_expr_after_arg_stage) {
    f.out = alloc_expr_.alloc(expr{expr::app{f.fun, f.arg}});
    return std::nullopt;
}

#endif
