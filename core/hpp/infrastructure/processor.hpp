#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <memory>
#include <optional>
#include <utility>
#include <variant>
#include "value_objects/detach_expr_after_arg_stage.hpp"
#include "value_objects/detach_expr_after_body_stage.hpp"
#include "value_objects/detach_expr_after_fun_stage.hpp"
#include "value_objects/detach_expr_frame.hpp"
#include "value_objects/detach_expr_funcall.hpp"
#include "value_objects/detach_expr_stage.hpp"
#include "value_objects/detach_expr_start_stage.hpp"
#include "value_objects/main_after_detach_stage.hpp"
#include "value_objects/main_detach_stage.hpp"
#include "value_objects/main_frame.hpp"
#include "value_objects/main_funcall.hpp"
#include "value_objects/main_reify_stage.hpp"
#include "value_objects/main_stage.hpp"
#include "value_objects/app_after_body_stage.hpp"
#include "value_objects/app_after_fun_stage.hpp"
#include "value_objects/app_need_fun_stage.hpp"
#include "value_objects/continuation.hpp"
#include "value_objects/funcall.hpp"
#include "value_objects/reduce_app_continuation.hpp"
#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_app_stage.hpp"
#include "value_objects/reduce_var_continuation.hpp"
#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_var_stage.hpp"
#include "value_objects/reduce_whnf_continuation.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reduce_whnf_stage.hpp"
#include "value_objects/reify_clo_after_body_stage.hpp"
#include "value_objects/reify_clo_continuation.hpp"
#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_clo_need_body_stage.hpp"
#include "value_objects/reify_clo_stage.hpp"
#include "value_objects/reify_napp_after_arg_stage.hpp"
#include "value_objects/reify_napp_continuation.hpp"
#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_napp_need_arg_stage.hpp"
#include "value_objects/reify_napp_need_head_stage.hpp"
#include "value_objects/reify_napp_stage.hpp"
#include "value_objects/reify_val_after_quote_stage.hpp"
#include "value_objects/reify_val_after_whnf_stage.hpp"
#include "value_objects/reify_val_continuation.hpp"
#include "value_objects/reify_val_frame.hpp"
#include "value_objects/reify_val_need_whnf_stage.hpp"
#include "value_objects/reify_val_stage.hpp"
#include "value_objects/var_after_force_stage.hpp"
#include "value_objects/var_start_stage.hpp"
#include "value_objects/whnf_after_child_stage.hpp"
#include "value_objects/whnf_start_stage.hpp"

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach> struct processor {
    processor(IProcessReduce& process_reduce, IProcessReify& process_reify,
              IProcessEntrypoint& process_entrypoint, IProcessDetach& process_detach);
    continuation init_continuation(main_funcall fc);
    continuation init_continuation(reduce_whnf_funcall fc);
    continuation init_continuation(reduce_app_funcall fc);
    continuation init_continuation(reduce_var_funcall fc);
    continuation init_continuation(reify_val_funcall fc);
    continuation init_continuation(reify_clo_funcall fc);
    continuation init_continuation(reify_napp_funcall fc);
    continuation init_continuation(detach_expr_funcall fc);
    std::optional<std::pair<main_stage, funcall>>
    process(main_frame& f, main_reify_stage s);
    std::optional<std::pair<main_stage, funcall>>
    process(main_frame& f, main_detach_stage s);
    std::optional<std::pair<main_stage, funcall>>
    process(main_frame& f, main_after_detach_stage s);
    std::optional<std::pair<reduce_whnf_stage, funcall>>
    process(reduce_whnf_frame& f, whnf_start_stage s);
    std::optional<std::pair<reduce_whnf_stage, funcall>>
    process(reduce_whnf_frame& f, whnf_after_child_stage s);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_need_fun_stage s);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_after_fun_stage s);
    std::optional<std::pair<reduce_app_stage, funcall>>
    process(reduce_app_frame& f, app_after_body_stage s);
    std::optional<std::pair<reduce_var_stage, funcall>>
    process(reduce_var_frame& f, var_start_stage s);
    std::optional<std::pair<reduce_var_stage, funcall>>
    process(reduce_var_frame& f, var_after_force_stage s);
    std::optional<std::pair<reify_val_stage, funcall>>
    process(reify_val_frame& f, reify_val_need_whnf_stage s);
    std::optional<std::pair<reify_val_stage, funcall>>
    process(reify_val_frame& f, reify_val_after_whnf_stage s);
    std::optional<std::pair<reify_val_stage, funcall>>
    process(reify_val_frame& f, reify_val_after_quote_stage s);
    std::optional<std::pair<reify_clo_stage, funcall>>
    process(reify_clo_frame& f, reify_clo_need_body_stage s);
    std::optional<std::pair<reify_clo_stage, funcall>>
    process(reify_clo_frame& f, reify_clo_after_body_stage s);
    std::optional<std::pair<reify_napp_stage, funcall>>
    process(reify_napp_frame& f, reify_napp_need_head_stage s);
    std::optional<std::pair<reify_napp_stage, funcall>>
    process(reify_napp_frame& f, reify_napp_need_arg_stage s);
    std::optional<std::pair<reify_napp_stage, funcall>>
    process(reify_napp_frame& f, reify_napp_after_arg_stage s);
    std::optional<std::pair<detach_expr_stage, funcall>>
    process(detach_expr_frame& f, detach_expr_start_stage s);
    std::optional<std::pair<detach_expr_stage, funcall>>
    process(detach_expr_frame& f, detach_expr_after_body_stage s);
    std::optional<std::pair<detach_expr_stage, funcall>>
    process(detach_expr_frame& f, detach_expr_after_fun_stage s);
    std::optional<std::pair<detach_expr_stage, funcall>>
    process(detach_expr_frame& f, detach_expr_after_arg_stage s);

  private:
    IProcessReduce& process_reduce_;
    IProcessReify& process_reify_;
    IProcessEntrypoint& process_entrypoint_;
    IProcessDetach& process_detach_;
};

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::processor(
    IProcessReduce& process_reduce, IProcessReify& process_reify,
    IProcessEntrypoint& process_entrypoint, IProcessDetach& process_detach)
    : process_reduce_(process_reduce), process_reify_(process_reify),
      process_entrypoint_(process_entrypoint), process_detach_(process_detach) {
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(
    main_funcall fc) {
    return process_entrypoint_.init_continuation(std::move(fc));
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reduce_whnf_funcall fc) {
    return reduce_whnf_continuation{reduce_whnf_frame{fc.slot},
                                    whnf_start_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reduce_app_funcall fc) {
    return reduce_app_continuation{
        reduce_app_frame{fc.slot, fc.application, fc.e, std::shared_ptr<val>{}},
        app_need_fun_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reduce_var_funcall fc) {
    return reduce_var_continuation{
        reduce_var_frame{fc.slot, fc.variable, fc.e, nullptr},
        var_start_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reify_val_funcall fc) {
    return reify_val_continuation{reify_val_frame{fc.out, fc.v, fc.depth},
                                  reify_val_need_whnf_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reify_clo_funcall fc) {
    return reify_clo_continuation{
        reify_clo_frame{fc.out, fc.closure, fc.depth, std::shared_ptr<expr>{},
                        std::shared_ptr<val>{}},
        reify_clo_need_body_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(reify_napp_funcall fc) {
    return reify_napp_continuation{
        reify_napp_frame{fc.out, fc.neutral, fc.depth, std::shared_ptr<expr>{},
                         std::shared_ptr<expr>{}, std::shared_ptr<val>{},
                         std::shared_ptr<val>{}},
        reify_napp_need_head_stage{}};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
continuation
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::init_continuation(
    detach_expr_funcall fc) {
    return process_detach_.init_continuation(std::move(fc));
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_whnf_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_whnf_frame& f,
                                         whnf_start_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_whnf_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_whnf_frame& f,
                                         whnf_after_child_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_app_frame& f,
                                         app_need_fun_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_app_frame& f,
                                         app_after_fun_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_app_frame& f,
                                         app_after_body_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_var_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_var_frame& f,
                                         var_start_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reduce_var_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reduce_var_frame& f,
                                         var_after_force_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_val_frame& f,
                                         reify_val_need_whnf_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_val_frame& f,
                                         reify_val_after_whnf_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_val_frame& f,
                                         reify_val_after_quote_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_clo_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_clo_frame& f,
                                         reify_clo_need_body_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_clo_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_clo_frame& f,
                                         reify_clo_after_body_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_napp_frame& f,
                                         reify_napp_need_head_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_napp_frame& f,
                                         reify_napp_need_arg_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(reify_napp_frame& f,
                                         reify_napp_after_arg_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<main_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    main_frame& f, main_reify_stage s) {
    return process_entrypoint_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<main_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    main_frame& f, main_detach_stage s) {
    return process_entrypoint_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<main_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    main_frame& f, main_after_detach_stage s) {
    return process_entrypoint_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<detach_expr_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    detach_expr_frame& f, detach_expr_start_stage s) {
    auto result = process_detach_.process(f, s);
    if(!result.has_value())
        return std::nullopt;
    return std::pair<detach_expr_stage, funcall>{std::move(result->first),
                                                 std::move(result->second)};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<detach_expr_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    detach_expr_frame& f, detach_expr_after_body_stage s) {
    auto result = process_detach_.process(f, s);
    if(!result.has_value())
        return std::nullopt;
    return std::pair<detach_expr_stage, funcall>{std::move(result->first),
                                                 std::move(result->second)};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<detach_expr_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    detach_expr_frame& f, detach_expr_after_fun_stage s) {
    auto result = process_detach_.process(f, s);
    if(!result.has_value())
        return std::nullopt;
    return std::pair<detach_expr_stage, funcall>{std::move(result->first),
                                                 std::move(result->second)};
}

template <typename IProcessReduce, typename IProcessReify, typename IProcessEntrypoint, typename IProcessDetach>
std::optional<std::pair<detach_expr_stage, funcall>>
processor<IProcessReduce, IProcessReify, IProcessEntrypoint, IProcessDetach>::process(
    detach_expr_frame& f, detach_expr_after_arg_stage s) {
    auto result = process_detach_.process(f, s);
    if(!result.has_value())
        return std::nullopt;
    return std::pair<detach_expr_stage, funcall>{std::move(result->first),
                                                 std::move(result->second)};
}


#endif
