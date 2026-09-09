#ifndef COMMAND_RUNNER_HPP
#define COMMAND_RUNNER_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Builder.hpp"

struct BuildOptions {
  int jobCount;
  BuildMode buildMode;
};

class CommandRunner {
 public:
  void runPrintHelp(const char *programName);

  std::optional<BuildOptions> parseBuildOptions(int argc, char *argv[]) const;

  int runListCommand(
      const std::optional<std::unordered_map<std::string, BuildTarget>>
          &targets);

  int runBuild(
      Builder &builder,
      const std::unordered_map<std::string, BuildTarget> &targetMap,
      const std::vector<std::string> &buildOrder, int jobCount,
      BuildMode buildMode);

  int runClean(Builder &builder);

 private:
  std::optional<int> parseJobCount(const std::string &optionValue) const;
  std::optional<BuildMode> parseBuildMode(
      const std::string &optionValue) const;
};

#endif  // COMMAND_RUNNER_HPP
