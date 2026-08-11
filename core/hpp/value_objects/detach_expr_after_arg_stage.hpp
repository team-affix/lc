#ifndef DETACH_EXPR_AFTER_ARG_STAGE_HPP
#define DETACH_EXPR_AFTER_ARG_STAGE_HPP

struct detach_expr_after_arg_stage {
    auto operator<=>(const detach_expr_after_arg_stage&) const = default;
};

#endif
