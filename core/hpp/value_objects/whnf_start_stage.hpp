#ifndef WHNF_START_STAGE_HPP
#define WHNF_START_STAGE_HPP

struct whnf_start_stage {
    auto operator<=>(const whnf_start_stage&) const = default;
};

#endif
