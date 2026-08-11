#ifndef EXPR_HPP
#define EXPR_HPP

#include <cstdint>
#include <memory>
#include <variant>

struct expr {
    struct var {
        uint32_t index;
    };
    struct abs {
        std::shared_ptr<expr> body;
    };
    struct app {
        std::shared_ptr<expr> fun;
        std::shared_ptr<expr> arg;
    };
    std::variant<var, abs, app> content;
};

#endif
