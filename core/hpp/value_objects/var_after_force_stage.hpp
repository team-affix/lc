#ifndef VAR_AFTER_FORCE_STAGE_HPP
#define VAR_AFTER_FORCE_STAGE_HPP

struct var_after_force_stage {
    auto operator<=>(const var_after_force_stage&) const = default;
};

#endif
