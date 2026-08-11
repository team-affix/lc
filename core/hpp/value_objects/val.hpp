#ifndef VAL_HPP
#define VAL_HPP

#include <cstdint>
#include <memory>
#include <variant>
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"

struct val {
    struct clo {
        std::shared_ptr<expr> term;
        std::shared_ptr<env> environment;
    };
    struct fvar {
        uint32_t depth;
    };
    struct napp {
        std::shared_ptr<val> head;
        std::shared_ptr<expr> arg;
        std::shared_ptr<env> arg_env;
    };
    std::variant<clo, fvar, napp> content;
};

#endif
