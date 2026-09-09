#ifndef COMMAND_RUNNER_HPP
#define COMMAND_RUNNER_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Builder.hpp"

class CommandRunner {
 public:
  void runPrintHelp(const char *programName);

  int parseBuildMode(const std::string &optionValue, BuildMode &buildMode);

  int runListCommand(
      const std::optional<std::unordered_map<std::string, BuildTarget>>
          &targets);

  int runBuild(
      Builder &builder,
      const std::unordered_map<std::string, BuildTarget> &targetMap,
      const std::vector<std::string> &buildOrder, int jobCount,
      BuildMode buildMode);

  int runClean(Builder &builder);
};

#endif  // COMMAND_RUNNER_HPP
