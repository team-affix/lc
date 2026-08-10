#ifndef REIFY_NAPP_AFTER_ARG_STAGE_HPP
#define REIFY_NAPP_AFTER_ARG_STAGE_HPP

struct reify_napp_after_arg_stage {
    auto operator<=>(const reify_napp_after_arg_stage&) const = default;
};

#endif
