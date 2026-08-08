#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "debug_assert.hpp"
#include "value_objects/frame.hpp"
#include "value_objects/reduce_app_frame.hpp"
#include "value_objects/reduce_var_frame.hpp"
#include "value_objects/reduce_whnf_frame.hpp"
#include "value_objects/reify_clo_frame.hpp"
#include "value_objects/reify_napp_frame.hpp"
#include "value_objects/reify_val_frame.hpp"
#include <optional>
#include <variant>

template <typename IProcessReduceWhnf, typename IProcessReduceApp,
          typename IProcessReduceVar, typename IProcessReifyVal,
          typename IProcessReifyClo, typename IProcessReifyNapp>
struct processor {
    processor(IProcessReduceWhnf& process_reduce_whnf,
              IProcessReduceApp& process_reduce_app,
              IProcessReduceVar& process_reduce_var,
              IProcessReifyVal& process_reify_val,
              IProcessReifyClo& process_reify_clo,
              IProcessReifyNapp& process_reify_napp);
    std::optional<frame> process(frame& f);

  private:
    IProcessReduceWhnf& process_reduce_whnf_;
    IProcessReduceApp& process_reduce_app_;
    IProcessReduceVar& process_reduce_var_;
    IProcessReifyVal& process_reify_val_;
    IProcessReifyClo& process_reify_clo_;
    IProcessReifyNapp& process_reify_napp_;
};

template <typename IProcessReduceWhnf, typename IProcessReduceApp,
          typename IProcessReduceVar, typename IProcessReifyVal,
          typename IProcessReifyClo, typename IProcessReifyNapp>
processor<IProcessReduceWhnf, IProcessReduceApp, IProcessReduceVar,
          IProcessReifyVal, IProcessReifyClo,
          IProcessReifyNapp>::processor(IProcessReduceWhnf& process_reduce_whnf,
                                        IProcessReduceApp& process_reduce_app,
                                        IProcessReduceVar& process_reduce_var,
                                        IProcessReifyVal& process_reify_val,
                                        IProcessReifyClo& process_reify_clo,
                                        IProcessReifyNapp& process_reify_napp)
    : process_reduce_whnf_(process_reduce_whnf),
      process_reduce_app_(process_reduce_app),
      process_reduce_var_(process_reduce_var),
      process_reify_val_(process_reify_val),
      process_reify_clo_(process_reify_clo),
      process_reify_napp_(process_reify_napp) {
}

template <typename IProcessReduceWhnf, typename IProcessReduceApp,
          typename IProcessReduceVar, typename IProcessReifyVal,
          typename IProcessReifyClo, typename IProcessReifyNapp>
std::optional<frame>
processor<IProcessReduceWhnf, IProcessReduceApp, IProcessReduceVar,
          IProcessReifyVal, IProcessReifyClo, IProcessReifyNapp>::process(
    frame& f) {
    if(reduce_whnf_frame* whnf = std::get_if<reduce_whnf_frame>(&f))
        return process_reduce_whnf_.process_whnf(*whnf);
    if(reduce_app_frame* app = std::get_if<reduce_app_frame>(&f))
        return process_reduce_app_.process_app(*app);
    if(reduce_var_frame* var = std::get_if<reduce_var_frame>(&f))
        return process_reduce_var_.process_var(*var);
    if(reify_val_frame* val = std::get_if<reify_val_frame>(&f))
        return process_reify_val_.process_val(*val);
    if(reify_clo_frame* clo = std::get_if<reify_clo_frame>(&f))
        return process_reify_clo_.process_clo(*clo);
    reify_napp_frame* napp = std::get_if<reify_napp_frame>(&f);
    DEBUG_ASSERT(napp != nullptr);
    return process_reify_napp_.process_napp(*napp);
}

#endif
