#ifndef REIFY_VAL_AFTER_WHNF_STAGE_HPP
#define REIFY_VAL_AFTER_WHNF_STAGE_HPP

struct reify_val_after_whnf_stage {
    auto operator<=>(const reify_val_after_whnf_stage&) const = default;
};

#endif
