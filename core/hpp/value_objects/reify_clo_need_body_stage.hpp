#ifndef REIFY_CLO_NEED_BODY_STAGE_HPP
#define REIFY_CLO_NEED_BODY_STAGE_HPP

struct reify_clo_need_body_stage {
    auto operator<=>(const reify_clo_need_body_stage&) const = default;
};

#endif
