#ifndef COMMAND_RUNNER_HPP
#define COMMAND_RUNNER_HPP

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Builder.hpp"
#include "ConfigParser.hpp"

struct BuildOptions {
  int jobCount;
  BuildMode buildMode;
};

class CommandRunner {
 public:
  void runPrintHelp(const char *programName);

  int runBuildCommand(Builder &builder, const ConfigParser &configParser,
                      const std::filesystem::path &configPath, int argc,
                      char *argv[]);

  int runListCommand(
      const std::optional<std::unordered_map<std::string, BuildTarget>>
          &targets);

  int runClean(Builder &builder);

 private:
  std::optional<BuildOptions> parseBuildOptions(int argc, char *argv[]) const;
  std::optional<int> parseJobCount(const std::string &optionValue) const;
  std::optional<BuildMode> parseBuildMode(const std::string &optionValue) const;

  int runBuild(Builder &builder,
               const std::unordered_map<std::string, BuildTarget> &targetMap,
               const std::vector<std::string> &buildOrder, int jobCount,
               BuildMode buildMode);
};

#endif  // COMMAND_RUNNER_HPP
