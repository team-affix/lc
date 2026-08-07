#include "value_objects/expr_hash.hpp"

#include <functional>
#include <variant>

size_t expr_hash::operator()(const expr& expression) const noexcept {
    if (const expr::var* variable = std::get_if<expr::var>(&expression.content)) {
        size_t seed = std::hash<uint32_t>{}(variable->index);
        return hash_combine(seed, 1);
    }
    if (const expr::abs* abstraction = std::get_if<expr::abs>(&expression.content)) {
        size_t seed = std::hash<const expr*>{}(abstraction->body);
        return hash_combine(seed, 2);
    }
    const expr::app& application = std::get<expr::app>(expression.content);
    size_t seed = std::hash<const expr*>{}(application.fun);
    seed = hash_combine(seed, std::hash<const expr*>{}(application.arg));
    return hash_combine(seed, 3);
}

size_t expr_hash::hash_combine(size_t seed, size_t value) noexcept {
    return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}
