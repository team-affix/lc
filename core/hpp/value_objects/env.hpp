#ifndef ENV_HPP
#define ENV_HPP

#include <variant>
#include "value_objects/expr.hpp"

struct val;

struct env {
    struct delayed {
        const expr* arg;
        env* arg_env;
        auto operator<=>(const delayed&) const = default;
    };
    struct ready {
        const val* value;
        auto operator<=>(const ready&) const = default;
    };
    std::variant<delayed, ready> binder;
    env* parent;
    auto operator<=>(const env&) const = default;
};

#endif
