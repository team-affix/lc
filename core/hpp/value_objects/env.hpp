#ifndef ENV_HPP
#define ENV_HPP

struct val;

struct env {
    const val* bound_value;
    env* parent;
    auto operator<=>(const env&) const = default;
};

#endif
