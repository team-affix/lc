#ifndef MAIN_DETACH_STAGE_HPP
#define MAIN_DETACH_STAGE_HPP

struct main_detach_stage {
    auto operator<=>(const main_detach_stage&) const = default;
};

#endif
