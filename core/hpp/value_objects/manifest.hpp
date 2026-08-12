#ifndef MANIFEST_HPP
#define MANIFEST_HPP

#include <memory>
#include <optional>
#include "infrastructure/entrypoint_processor.hpp"
#include "infrastructure/env_lookup.hpp"
#include "infrastructure/env_factory.hpp"
#include "infrastructure/expr_factory.hpp"
#include "infrastructure/interpreter.hpp"
#include "infrastructure/initial_frame_generator.hpp"
#include "infrastructure/output_detacher.hpp"
#include "infrastructure/processor.hpp"
#include "infrastructure/rc_pool.hpp"
#include "infrastructure/reduction_processor.hpp"
#include "infrastructure/reification_processor.hpp"
#include "infrastructure/space_usage_calculator.hpp"
#include "infrastructure/val_factory.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/val.hpp"

struct manifest {
    using expr_nodes_t = rc_pool<expr>;
    using val_nodes_t = rc_pool<val>;
    using env_nodes_t = rc_pool<env>;
    using expr_factory_t = ::expr_factory<expr_nodes_t>;
    using val_factory_t = ::val_factory<val_nodes_t>;
    using env_factory_t = ::env_factory<env_nodes_t>;
    using reduction_processor_t =
        ::reduction_processor<val_factory_t, val_factory_t, env_factory_t,
                              env_lookup>;
    using reification_processor_t =
        ::reification_processor<expr_factory_t, expr_factory_t, expr_factory_t,
                                val_factory_t, env_factory_t, val_factory_t>;
    using entrypoint_processor_t = ::entrypoint_processor;
    using output_detacher_t = ::output_detacher<expr_nodes_t>;
    using processor_t =
        ::processor<reduction_processor_t, reification_processor_t,
                    entrypoint_processor_t, output_detacher_t>;
    using initial_frame_generator_t = ::initial_frame_generator<val_factory_t>;
    using interpreter_t =
        ::interpreter<continuation, funcall, processor_t, processor_t>;
    using space_usage_calculator_t =
        ::space_usage_calculator<expr_nodes_t, val_nodes_t, env_nodes_t,
                                 interpreter_t>;

    manifest(std::shared_ptr<expr>& out, std::shared_ptr<expr> term,
             expr_nodes_t& out_nodes);

    expr_nodes_t expr_nodes;
    val_nodes_t val_nodes;
    env_nodes_t env_nodes;
    expr_factory_t exprs;
    env_factory_t envs;
    val_factory_t vals;
    output_detacher_t detacher;
    entrypoint_processor_t entrypoint;
    env_lookup lookup;
    reduction_processor_t red;
    reification_processor_t re;
    processor_t proc;
    initial_frame_generator_t initial_frame_gen;
    std::optional<interpreter_t> interp;
    space_usage_calculator_t space_usage_calculator;
};

#endif
