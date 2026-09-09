#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "BuildMode.hpp"
#include "Builder.hpp"
#include "CommandRunner.hpp"
#include "ConfigParser.hpp"
#include "DependencyGraph.hpp"
#include "ExitCodes.hpp"

namespace {

constexpr char CONFIG_PATH[] = "dagbuild.conf";
constexpr int DEFAULT_JOB_COUNT = 1;
constexpr int MINIMUM_JOB_COUNT = 1;
constexpr int MAXIMUM_JOB_COUNT = 10;

}  // namespace

int main(int argc, char *argv[]) {
  Builder builder;
  ConfigParser configParser;
  CommandRunner commandRunner;

  if (argc < 2) {
    std::cerr << "Error: no command provided.\n";
    std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
    return ExitCode::INVALID_COMMAND;
  }

  const std::string command = argv[1];

  if (command == "help") {
    commandRunner.runPrintHelp(argv[0]);
    return ExitCode::SUCCESS;
  }

  if (command == "build") {
    if (argc != 3 && argc != 5 && argc != 7) {
      std::cerr << "Usage: " << argv[0]
                << " build <target> [--jobs <number>] "
                   "[--mode <debug|release>]\n";
      return ExitCode::INVALID_COMMAND;
    }

    int jobCount = DEFAULT_JOB_COUNT;
    BuildMode buildMode = BuildMode::Debug;

    for (int i = 3; i < argc; i += 2) {
      const std::string option = argv[i];
      const std::string optionValue = argv[i + 1];

      if (option == "--jobs") {
        try {
          jobCount = std::stoi(optionValue);
        } catch (const std::exception &) {
          std::cerr << "Error: jobs must be a number.\n";
          return ExitCode::INVALID_COMMAND;
        }

        if (jobCount < MINIMUM_JOB_COUNT || jobCount > MAXIMUM_JOB_COUNT) {
          std::cerr << "Error: jobs must be between " << MINIMUM_JOB_COUNT
                    << " and " << MAXIMUM_JOB_COUNT << ".\n";
          return ExitCode::INVALID_COMMAND;
        }
      } else if (option == "--mode") {
        const int modeResult =
            commandRunner.parseBuildMode(optionValue, buildMode);
        if (modeResult != ExitCode::SUCCESS) {
          return modeResult;
        }
      } else {
        std::cerr << "Error: expected '--jobs' or '--mode'.\n";
        return ExitCode::INVALID_COMMAND;
      }
    }

    std::cout << "Build mode: "
              << (buildMode == BuildMode::Debug ? "debug" : "release") << '\n';

    const auto parsedTargets = configParser.parseTargets(CONFIG_PATH);
    if (!parsedTargets.has_value()) {
      return ExitCode::CONFIGURATION_ERROR;
    }

    const std::string requestedTarget = argv[2];
    const auto &targetMap = parsedTargets.value();

    DependencyGraph dependencyGraph(targetMap);
    const auto buildOrder = dependencyGraph.createBuildOrder(requestedTarget);
    if (!buildOrder) {
      return ExitCode::CONFIGURATION_ERROR;
    }

    return commandRunner.runBuild(builder, targetMap, buildOrder.value(),
                                  jobCount, buildMode);
  }

  if (command == "clean") {
    if (argc != 2) {
      std::cerr << "Error: clean does not accept additional arguments.\n";
      return ExitCode::INVALID_COMMAND;
    }
    return commandRunner.runClean(builder);
  }

  if (command == "list") {
    if (argc != 2) {
      std::cerr << "Error: list does not accept additional arguments.\n";
      return ExitCode::INVALID_COMMAND;
    }
    const auto targets = configParser.parseTargets(CONFIG_PATH);
    return commandRunner.runListCommand(targets);
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return ExitCode::INVALID_COMMAND;
}
