#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include <string>
#include <unordered_map>

#include "BuildTarget.hpp"

enum class VisitState { NotVisited, Visiting, Visited };

class DependencyGraph {
 private:
  const std::unordered_map<std::string, BuildTarget> &targets_;

  bool dfs(const std::string &currentTargetName,
           std::unordered_map<std::string, VisitState> &states,
           std::vector<std::string> &buildOrder);

 public:
  DependencyGraph(const std::unordered_map<std::string, BuildTarget> &targets);

  std::optional<std::vector<std::string>> createBuildOrder(
      const std::string &requestedTarget);
};

#endif