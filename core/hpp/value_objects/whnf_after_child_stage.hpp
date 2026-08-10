#ifndef WHNF_AFTER_CHILD_STAGE_HPP
#define WHNF_AFTER_CHILD_STAGE_HPP

struct whnf_after_child_stage {
    auto operator<=>(const whnf_after_child_stage&) const = default;
};

#endif
