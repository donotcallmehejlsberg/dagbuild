#ifndef COMMAND_RUNNER_HPP
#define COMMAND_RUNNER_HPP

#include <optional>
#include <unordered_map>

#include "Builder.hpp"

class CommandRunner {
 public:
  int runListCommand(
      const std::optional<std::unordered_map<std::string, BuildTarget>>
          &targets);

  int runBuildCommand(
      Builder &builder,
      const std::unordered_map<std::string, BuildTarget> &targetMap,
      const std::vector<std::string> &buildOrder, int jobCount,
      BuildMode buildMode);
};

#endif  // COMMAND_RUNNER_HPP
