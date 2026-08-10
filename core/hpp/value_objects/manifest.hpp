#ifndef MANIFEST_HPP
#define MANIFEST_HPP

#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/expr.hpp"

struct manifest {
    using reducer_t = ::reducer<val_pool, val_pool, env_pool, env_lookup>;
    using reifier_t = ::reifier<expr_pool, expr_pool, expr_pool, val_pool,
                                env_pool, val_pool>;
    using processor_t = ::processor<reducer_t, reifier_t>;
    using normalizer_t = ::normalizer<val_pool>;
    using interpreter_t =
        ::interpreter<continuation, processor_t, processor_t>;

    manifest(const expr*& out, const expr* term);

    expr_pool exprs;
    env_pool envs;
    val_pool vals;
    env_lookup lookup;
    reducer_t red;
    reifier_t re;
    processor_t proc;
    normalizer_t norm;
    interpreter_t interp;
};

#endif
