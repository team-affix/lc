#include "infrastructure/output_detacher.hpp"

#include <variant>

detach_expr_continuation
output_detacher::init_continuation(detach_expr_funcall fc) {
    return detach_expr_continuation{
        detach_expr_frame{fc.out, std::move(fc.src), std::shared_ptr<expr>{},
                          std::shared_ptr<expr>{}, std::shared_ptr<expr>{}},
        detach_expr_start_stage{}};
}

std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher::process(detach_expr_frame& f, detach_expr_start_stage) {
    if(const expr::var* v = std::get_if<expr::var>(&f.src->content)) {
        f.out = std::make_shared<expr>(expr{expr::var{v->index}});
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

std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher::process(detach_expr_frame& f, detach_expr_after_body_stage) {
    f.out = std::make_shared<expr>(expr{expr::abs{f.body}});
    return std::nullopt;
}

std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher::process(detach_expr_frame& f, detach_expr_after_fun_stage) {
    const expr::app& ap = std::get<expr::app>(f.src->content);
    return std::pair<detach_expr_stage, detach_expr_funcall>{
        detach_expr_after_arg_stage{}, detach_expr_funcall{f.arg, ap.arg}};
}

std::optional<std::pair<detach_expr_stage, detach_expr_funcall>>
output_detacher::process(detach_expr_frame& f, detach_expr_after_arg_stage) {
    f.out = std::make_shared<expr>(expr{expr::app{f.fun, f.arg}});
    return std::nullopt;
}
