#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

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
#include <optional>
#include <utility>
#include <variant>

template <typename IReducer, typename IReifier> struct processor {
    processor(IReducer& reducer, IReifier& reifier);
    continuation init_continuation(reduce_whnf_funcall fc);
    continuation init_continuation(reduce_app_funcall fc);
    continuation init_continuation(reduce_var_funcall fc);
    continuation init_continuation(reify_val_funcall fc);
    continuation init_continuation(reify_clo_funcall fc);
    continuation init_continuation(reify_napp_funcall fc);
    std::pair<reduce_whnf_stage, std::optional<funcall>>
    process(reduce_whnf_frame& f, whnf_start_stage s);
    std::pair<reduce_whnf_stage, std::optional<funcall>>
    process(reduce_whnf_frame& f, whnf_after_child_stage s);
    std::pair<reduce_app_stage, std::optional<funcall>>
    process(reduce_app_frame& f, app_need_fun_stage s);
    std::pair<reduce_app_stage, std::optional<funcall>>
    process(reduce_app_frame& f, app_after_fun_stage s);
    std::pair<reduce_app_stage, std::optional<funcall>>
    process(reduce_app_frame& f, app_after_body_stage s);
    std::pair<reduce_var_stage, std::optional<funcall>>
    process(reduce_var_frame& f, var_start_stage s);
    std::pair<reduce_var_stage, std::optional<funcall>>
    process(reduce_var_frame& f, var_after_force_stage s);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_need_whnf_stage s);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_after_whnf_stage s);
    std::pair<reify_val_stage, std::optional<funcall>>
    process(reify_val_frame& f, reify_val_after_quote_stage s);
    std::pair<reify_clo_stage, std::optional<funcall>>
    process(reify_clo_frame& f, reify_clo_need_body_stage s);
    std::pair<reify_clo_stage, std::optional<funcall>>
    process(reify_clo_frame& f, reify_clo_after_body_stage s);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_need_head_stage s);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_need_arg_stage s);
    std::pair<reify_napp_stage, std::optional<funcall>>
    process(reify_napp_frame& f, reify_napp_after_arg_stage s);

  private:
    IReducer& reducer_;
    IReifier& reifier_;
};

template <typename IReducer, typename IReifier>
processor<IReducer, IReifier>::processor(IReducer& reducer, IReifier& reifier)
    : reducer_(reducer), reifier_(reifier) {
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reduce_whnf_funcall fc) {
    return reduce_whnf_continuation{reduce_whnf_frame{fc.slot},
                                    whnf_start_stage{}};
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reduce_app_funcall fc) {
    return reduce_app_continuation{
        reduce_app_frame{fc.slot, fc.application, fc.e, nullptr},
        app_need_fun_stage{}};
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reduce_var_funcall fc) {
    return reduce_var_continuation{
        reduce_var_frame{fc.slot, fc.variable, fc.e, nullptr},
        var_start_stage{}};
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reify_val_funcall fc) {
    return reify_val_continuation{reify_val_frame{fc.out, fc.v, fc.depth},
                                  reify_val_need_whnf_stage{}};
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reify_clo_funcall fc) {
    return reify_clo_continuation{
        reify_clo_frame{fc.out, fc.closure, fc.depth, nullptr, nullptr},
        reify_clo_need_body_stage{}};
}

template <typename IReducer, typename IReifier>
continuation
processor<IReducer, IReifier>::init_continuation(reify_napp_funcall fc) {
    return reify_napp_continuation{
        reify_napp_frame{fc.out, fc.neutral, fc.depth, nullptr, nullptr,
                         nullptr, nullptr},
        reify_napp_need_head_stage{}};
}

template <typename IReducer, typename IReifier>
std::pair<reduce_whnf_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_whnf_frame& f,
                                       whnf_start_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_whnf_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_whnf_frame& f,
                                       whnf_after_child_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_app_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_app_frame& f,
                                       app_need_fun_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_app_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_app_frame& f,
                                       app_after_fun_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_app_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_app_frame& f,
                                       app_after_body_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_var_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_var_frame& f, var_start_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reduce_var_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reduce_var_frame& f,
                                       var_after_force_stage s) {
    return reducer_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_val_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_val_frame& f,
                                       reify_val_need_whnf_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_val_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_val_frame& f,
                                       reify_val_after_whnf_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_val_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_val_frame& f,
                                       reify_val_after_quote_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_clo_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_clo_frame& f,
                                       reify_clo_need_body_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_clo_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_clo_frame& f,
                                       reify_clo_after_body_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_napp_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_napp_frame& f,
                                       reify_napp_need_head_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_napp_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_napp_frame& f,
                                       reify_napp_need_arg_stage s) {
    return reifier_.process(f, s);
}

template <typename IReducer, typename IReifier>
std::pair<reify_napp_stage, std::optional<funcall>>
processor<IReducer, IReifier>::process(reify_napp_frame& f,
                                       reify_napp_after_arg_stage s) {
    return reifier_.process(f, s);
}

#endif
