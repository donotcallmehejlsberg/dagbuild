#include "DependencyGraph.hpp"

DependencyGraph::DependencyGraph(
    const std::unordered_map<std::string, BuildTarget> &targets)
    : targets_(targets) {}

    
