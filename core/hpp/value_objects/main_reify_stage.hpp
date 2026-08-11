#ifndef MAIN_REIFY_STAGE_HPP
#define MAIN_REIFY_STAGE_HPP

struct main_reify_stage {
    auto operator<=>(const main_reify_stage&) const = default;
};

#endif
