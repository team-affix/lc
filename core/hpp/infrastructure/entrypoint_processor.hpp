#ifndef ENTRYPOINT_PROCESSOR_HPP
#define ENTRYPOINT_PROCESSOR_HPP

#include <memory>
#include <optional>
#include <utility>
#include "value_objects/detach_expr_funcall.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/main_after_detach_stage.hpp"
#include "value_objects/main_continuation.hpp"
#include "value_objects/main_detach_stage.hpp"
#include "value_objects/main_frame.hpp"
#include "value_objects/main_funcall.hpp"
#include "value_objects/main_reify_stage.hpp"
#include "value_objects/main_stage.hpp"
#include "value_objects/reify_val_funcall.hpp"
#include "infrastructure/expr_release.hpp"

struct entrypoint_processor {
    main_continuation init_continuation(main_funcall fc);
    std::optional<std::pair<main_stage, funcall>> process(main_frame& f,
                                                          main_reify_stage);
    std::optional<std::pair<main_stage, funcall>> process(main_frame& f,
                                                          main_detach_stage);
    std::optional<std::pair<main_stage, funcall>>
    process(main_frame& f, main_after_detach_stage);
};

inline main_continuation
entrypoint_processor::init_continuation(main_funcall fc) {
    return main_continuation{
        main_frame{fc.detached_out, std::shared_ptr<expr>{}, std::move(fc.seed)},
        main_reify_stage{}};
}

inline std::optional<std::pair<main_stage, funcall>>
entrypoint_processor::process(main_frame& f, main_reify_stage) {
    return std::pair<main_stage, funcall>{
        main_detach_stage{}, reify_val_funcall{f.pool_out, f.seed, 0}};
}

inline std::optional<std::pair<main_stage, funcall>>
entrypoint_processor::process(main_frame& f, main_detach_stage) {
    return std::pair<main_stage, funcall>{
        main_after_detach_stage{},
        detach_expr_funcall{f.detached_out, f.pool_out}};
}

inline std::optional<std::pair<main_stage, funcall>>
entrypoint_processor::process(main_frame& f, main_after_detach_stage) {
    f.detached_out = adopt_expr_tree(std::move(f.detached_out));
    f.pool_out.reset();
    return std::nullopt;
}

#endif
