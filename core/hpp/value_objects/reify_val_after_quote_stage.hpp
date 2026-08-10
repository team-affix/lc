#ifndef REIFY_VAL_AFTER_QUOTE_STAGE_HPP
#define REIFY_VAL_AFTER_QUOTE_STAGE_HPP

struct reify_val_after_quote_stage {
    auto operator<=>(const reify_val_after_quote_stage&) const = default;
};

#endif
