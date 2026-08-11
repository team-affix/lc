#include "value_objects/env.hpp"
#include "value_objects/val.hpp"

env::env() : bound_value(), parent() {
}

env::env(std::shared_ptr<val> bound_value, std::shared_ptr<env> parent)
    : bound_value(std::move(bound_value)), parent(std::move(parent)) {
}

env::env(const env& other)
    : bound_value(other.bound_value), parent(other.parent) {
}

env& env::operator=(const env& other) {
    bound_value = other.bound_value;
    parent = other.parent;
    return *this;
}

env::env(env&& other)
    : bound_value(std::move(other.bound_value)), parent(std::move(other.parent)) {
}

env& env::operator=(env&& other) {
    bound_value = std::move(other.bound_value);
    parent = std::move(other.parent);
    return *this;
}

env::~env() {
}
