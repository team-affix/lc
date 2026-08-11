#include "infrastructure/output_detacher.hpp"

#include <variant>

std::shared_ptr<expr>
output_detacher::prepare(const std::shared_ptr<expr>& e) const {
    if(const expr::var* v = std::get_if<expr::var>(&e->content))
        return std::make_shared<expr>(expr{expr::var{v->index}});
    if(const expr::abs* a = std::get_if<expr::abs>(&e->content))
        return std::make_shared<expr>(expr{expr::abs{prepare(a->body)}});
    const expr::app& ap = std::get<expr::app>(e->content);
    return std::make_shared<expr>(
        expr{expr::app{prepare(ap.fun), prepare(ap.arg)}});
}
