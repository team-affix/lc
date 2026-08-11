#ifndef GARBAGE_COLLECTOR_HPP
#define GARBAGE_COLLECTOR_HPP

template <typename ICollectOneExpr, typename ICollectOneVal,
          typename ICollectOneEnv>
struct garbage_collector {
    garbage_collector(ICollectOneExpr& collect_one_expr,
                      ICollectOneVal& collect_one_val,
                      ICollectOneEnv& collect_one_env);
    void collect();

  private:
    ICollectOneExpr& collect_one_expr_;
    ICollectOneVal& collect_one_val_;
    ICollectOneEnv& collect_one_env_;
};

template <typename ICollectOneExpr, typename ICollectOneVal,
          typename ICollectOneEnv>
garbage_collector<ICollectOneExpr, ICollectOneVal, ICollectOneEnv>::
    garbage_collector(ICollectOneExpr& collect_one_expr,
                      ICollectOneVal& collect_one_val,
                      ICollectOneEnv& collect_one_env)
    : collect_one_expr_(collect_one_expr), collect_one_val_(collect_one_val),
      collect_one_env_(collect_one_env) {
}

template <typename ICollectOneExpr, typename ICollectOneVal,
          typename ICollectOneEnv>
void garbage_collector<ICollectOneExpr, ICollectOneVal,
                       ICollectOneEnv>::collect() {
    while(collect_one_expr_.collect_one() || collect_one_val_.collect_one() ||
          collect_one_env_.collect_one())
        ;
}

#endif
