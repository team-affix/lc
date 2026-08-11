#ifndef MANIFEST_HPP
#define MANIFEST_HPP

#include <memory>
#include <optional>
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_pool.hpp"
#include "infrastructure/expr_pool.hpp"
#include "infrastructure/garbage_collector.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/normalizer.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/reducer.hpp"
#include "infrastructure/reifier.hpp"
#include "infrastructure/val_pool.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

struct manifest {
    using expr_nodes_t = rc_pool<expr>;
    using val_nodes_t = rc_pool<val>;
    using env_nodes_t = rc_pool<env>;
    using expr_pool_t = ::expr_pool<expr_nodes_t>;
    using val_pool_t = ::val_pool<val_nodes_t>;
    using env_pool_t = ::env_pool<env_nodes_t>;
    using reducer_t =
        ::reducer<val_pool_t, val_pool_t, env_pool_t, env_lookup>;
    using reifier_t =
        ::reifier<expr_pool_t, expr_pool_t, expr_pool_t, val_pool_t, env_pool_t,
                  val_pool_t>;
    using processor_t = ::processor<reducer_t, reifier_t>;
    using normalizer_t = ::normalizer<val_pool_t>;
    using interpreter_t =
        ::interpreter<continuation, processor_t, processor_t>;
    using garbage_collector_t =
        ::garbage_collector<expr_nodes_t, val_nodes_t, env_nodes_t>;

    manifest(std::shared_ptr<expr>& out, const expr* term);

    expr_nodes_t expr_nodes;
    val_nodes_t val_nodes;
    env_nodes_t env_nodes;
    expr_pool_t exprs;
    env_pool_t envs;
    val_pool_t vals;
    garbage_collector_t gc;
    env_lookup lookup;
    reducer_t red;
    reifier_t re;
    processor_t proc;
    normalizer_t norm;
    std::optional<interpreter_t> interp;
};

#endif
