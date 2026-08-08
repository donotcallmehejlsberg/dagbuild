#include "DependencyGraph.hpp"

#include <iostream>

DependencyGraph::DependencyGraph(
    const std::unordered_map<std::string, BuildTarget> &targets)
    : targets_(targets) {}

bool DependencyGraph::dfs(const std::string &currentTargetName,
                          std::unordered_map<std::string, VisitState> &states,
                          std::vector<std::string> &buildOrder) {
  const VisitState currentState = states.at(currentTargetName);
  if (currentState == VisitState::Visiting) {
    std::cerr << "Error: dependency cycle detected at target '"
              << currentTargetName << "'.\n";
    return false;
  }

  if (currentState == VisitState::Visited) {
    return true;
  }

  states.at(currentTargetName) = VisitState::Visiting;
  const auto targetIterator = targets_.find(currentTargetName);
  if (targetIterator == targets_.end()) {
    std::cerr << "Error: target '" << currentTargetName << "' not found.\n";
    return false;
  }

  const BuildTarget &currentTarget = targetIterator->second;
  for (auto &dependencyName : currentTarget.dependencyNames) {
    if (!dfs(dependencyName, states, buildOrder)) {
      return false;
    }
  }

  states.at(currentTargetName) = VisitState::Visited;
  buildOrder.push_back(currentTargetName);

  return true;
}
