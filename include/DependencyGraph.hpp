#ifndef DEPENDENCY_GRAPH_HPP
#define DEPENDENCY_GRAPH_HPP

#include <string>
#include <unordered_map>

#include "BuildTarget.hpp"

class DependencyGraph {
 private:
  const std::unordered_map<std::string, BuildTarget> &targets_;

 public:
  DependencyGraph(const std::unordered_map<std::string, BuildTarget> &targets);
};

#endif