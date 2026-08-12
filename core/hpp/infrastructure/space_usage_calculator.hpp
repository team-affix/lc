#ifndef SPACE_USAGE_CALCULATOR_HPP
#define SPACE_USAGE_CALCULATOR_HPP

#include <cstddef>
#include <optional>
#include "value_objects/continuation.hpp"
#include "value_objects/env.hpp"
#include "value_objects/expr.hpp"
#include "value_objects/val.hpp"

template <typename IExprNodesSize, typename IValNodesSize, typename IEnvNodesSize,
          typename IFrameCount>
struct space_usage_calculator {
    space_usage_calculator(IExprNodesSize& expr_nodes, IValNodesSize& val_nodes,
                           IEnvNodesSize& env_nodes, IFrameCount& frame_count);
    std::size_t space_usage() const;

  private:
    IExprNodesSize& expr_nodes_;
    IValNodesSize& val_nodes_;
    IEnvNodesSize& env_nodes_;
    IFrameCount& frame_count_;
};

template <typename IExprNodesSize, typename IValNodesSize, typename IEnvNodesSize,
          typename IFrameCount>
space_usage_calculator<IExprNodesSize, IValNodesSize, IEnvNodesSize,
                       IFrameCount>::space_usage_calculator(
    IExprNodesSize& expr_nodes, IValNodesSize& val_nodes,
    IEnvNodesSize& env_nodes, IFrameCount& frame_count)
    : expr_nodes_(expr_nodes)
    , val_nodes_(val_nodes)
    , env_nodes_(env_nodes)
    , frame_count_(frame_count) {
}

template <typename IExprNodesSize, typename IValNodesSize, typename IEnvNodesSize,
          typename IFrameCount>
std::size_t space_usage_calculator<IExprNodesSize, IValNodesSize, IEnvNodesSize,
                                   IFrameCount>::space_usage() const {
    return expr_nodes_.size() * sizeof(std::optional<expr>) +
           val_nodes_.size() * sizeof(std::optional<val>) +
           env_nodes_.size() * sizeof(std::optional<env>) +
           frame_count_.frame_count() * sizeof(continuation);
}

#endif
