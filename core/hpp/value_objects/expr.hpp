#ifndef EXPR_HPP
#define EXPR_HPP

#include <cstdint>
#include <variant>

struct expr {
    struct var {
        uint32_t index;
        auto operator<=>(const var&) const = default;
    };
    struct abs {
        const expr* body;
        auto operator<=>(const abs&) const = default;
    };
    struct app {
        const expr* fun;
        const expr* arg;
        auto operator<=>(const app&) const = default;
    };
    std::variant<var, abs, app> content;
    auto operator<=>(const expr&) const = default;
};

#endif
