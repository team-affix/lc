#ifndef REIFY_VAL_NEED_WHNF_STAGE_HPP
#define REIFY_VAL_NEED_WHNF_STAGE_HPP

struct reify_val_need_whnf_stage {
    auto operator<=>(const reify_val_need_whnf_stage&) const = default;
};

#endif
