#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <memory>
#include <optional>
#include <utility>
#include <variant>
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

template <typename IProcessReduce, typename IProcessReify> struct processor {
    processor(IProcessReduce& process_reduce, IProcessReify& process_reify);
    continuation init_continuation(reduce_whnf_funcall fc);
    continuation init_continuation(reduce_app_funcall fc);
    continuation init_continuation(reduce_var_funcall fc);
    continuation init_continuation(reify_val_funcall fc);
    continuation init_continuation(reify_clo_funcall fc);
    continuation init_continuation(reify_napp_funcall fc);
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

  private:
    IProcessReduce& process_reduce_;
    IProcessReify& process_reify_;
};

template <typename IProcessReduce, typename IProcessReify>
processor<IProcessReduce, IProcessReify>::processor(IProcessReduce& process_reduce,
                                           IProcessReify& process_reify)
    : process_reduce_(process_reduce), process_reify_(process_reify) {
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reduce_whnf_funcall fc) {
    return reduce_whnf_continuation{reduce_whnf_frame{fc.slot},
                                    whnf_start_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reduce_app_funcall fc) {
    return reduce_app_continuation{
        reduce_app_frame{fc.slot, fc.application, fc.e, std::shared_ptr<val>{}},
        app_need_fun_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reduce_var_funcall fc) {
    return reduce_var_continuation{
        reduce_var_frame{fc.slot, fc.variable, fc.e, nullptr},
        var_start_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reify_val_funcall fc) {
    return reify_val_continuation{reify_val_frame{fc.out, fc.v, fc.depth},
                                  reify_val_need_whnf_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reify_clo_funcall fc) {
    return reify_clo_continuation{
        reify_clo_frame{fc.out, fc.closure, fc.depth, std::shared_ptr<expr>{},
                        std::shared_ptr<val>{}},
        reify_clo_need_body_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
continuation
processor<IProcessReduce, IProcessReify>::init_continuation(reify_napp_funcall fc) {
    return reify_napp_continuation{
        reify_napp_frame{fc.out, fc.neutral, fc.depth, std::shared_ptr<expr>{},
                         std::shared_ptr<expr>{}, std::shared_ptr<val>{},
                         std::shared_ptr<val>{}},
        reify_napp_need_head_stage{}};
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_whnf_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_whnf_frame& f,
                                         whnf_start_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_whnf_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_whnf_frame& f,
                                         whnf_after_child_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_app_frame& f,
                                         app_need_fun_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_app_frame& f,
                                         app_after_fun_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_app_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_app_frame& f,
                                         app_after_body_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_var_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_var_frame& f,
                                         var_start_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reduce_var_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reduce_var_frame& f,
                                         var_after_force_stage s) {
    return process_reduce_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_val_frame& f,
                                         reify_val_need_whnf_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_val_frame& f,
                                         reify_val_after_whnf_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_val_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_val_frame& f,
                                         reify_val_after_quote_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_clo_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_clo_frame& f,
                                         reify_clo_need_body_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_clo_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_clo_frame& f,
                                         reify_clo_after_body_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_napp_frame& f,
                                         reify_napp_need_head_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_napp_frame& f,
                                         reify_napp_need_arg_stage s) {
    return process_reify_.process(f, s);
}

template <typename IProcessReduce, typename IProcessReify>
std::optional<std::pair<reify_napp_stage, funcall>>
processor<IProcessReduce, IProcessReify>::process(reify_napp_frame& f,
                                         reify_napp_after_arg_stage s) {
    return process_reify_.process(f, s);
}

#endif
