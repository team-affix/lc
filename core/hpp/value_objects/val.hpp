#ifndef VAL_HPP
#define VAL_HPP

#include <cstdint>
#include <variant>
#include "value_objects/expr.hpp"

struct env;

struct val {
    struct clo {
        const expr* body;
        env* captured;
        auto operator<=>(const clo&) const = default;
    };
    struct fvar {
        uint32_t depth;
        auto operator<=>(const fvar&) const = default;
    };
    struct napp {
        const val* head;
        const expr* arg;
        env* arg_env;
        auto operator<=>(const napp&) const = default;
    };
    std::variant<clo, fvar, napp> content;
    auto operator<=>(const val&) const = default;
};

#endif
