#ifndef ENV_HPP
#define ENV_HPP

#include <memory>

struct val;

struct env {
    env();
    env(std::shared_ptr<val> bound_value, std::shared_ptr<env> parent);
    env(const env& other);
    env& operator=(const env& other);
    env(env&& other);
    env& operator=(env&& other);
    ~env();

    std::shared_ptr<val> bound_value;
    std::shared_ptr<env> parent;
};

#endif
