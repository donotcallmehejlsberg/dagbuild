#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "BuildMode.hpp"
#include "Builder.hpp"
#include "ConfigParser.hpp"
#include "DependencyGraph.hpp"

namespace {

constexpr char CONFIG_PATH[] = "dagbuild.conf";
constexpr int DEFAULT_JOB_COUNT = 1;
constexpr int MINIMUM_JOB_COUNT = 1;
constexpr int MAXIMUM_JOB_COUNT = 10;

constexpr int SUCCESS_EXIT_CODE = 0;
constexpr int INVALID_COMMAND_EXIT_CODE = 1;
constexpr int CONFIGURATION_ERROR_EXIT_CODE = 2;
constexpr int BUILD_ERROR_EXIT_CODE = 3;

}  // namespace

void printHelp(const char *programName) {
  std::cout << "DAGBuild - a minimal C++ build system\n\n";
  std::cout << "Usage:\n";
  std::cout << "  " << programName << " build <target>\n";
  std::cout << "  " << programName << " build <target> --jobs <number>\n";
  std::cout << "  " << programName
            << " build <target> --mode <debug|release>\n";
  std::cout << "  " << programName
            << " build <target> --jobs <number> --mode <debug|release>\n";
  std::cout << "  " << programName << " clean\n";
  std::cout << "  " << programName << " list\n";
  std::cout << "  " << programName << " help\n";
}

int runListCommand(
    const std::optional<std::unordered_map<std::string, BuildTarget>>
        &targets) {
  if (!targets.has_value()) {
    return CONFIGURATION_ERROR_EXIT_CODE;
  }

  std::cout << "Available targets:\n";
  for (const auto &entry : targets.value()) {
    std::cout << "  " << entry.first << '\n';
  }

  return SUCCESS_EXIT_CODE;
}

int runBuildCommand(
    Builder &builder,
    const std::unordered_map<std::string, BuildTarget> &targetMap,
    const std::vector<std::string> &buildOrder, int jobCount,
    BuildMode buildMode) {
  for (const std::string &targetName : buildOrder) {
    const auto targetIterator = targetMap.find(targetName);

    if (targetIterator == targetMap.end()) {
      std::cerr << "Error: target '" << targetName << "' not found.\n";
      return CONFIGURATION_ERROR_EXIT_CODE;
    }

    BuildTarget target = targetIterator->second;
    const std::string modeDirectory =
        buildMode == BuildMode::Debug ? "debug" : "release";
    target.objectsDirectory /= modeDirectory;

    target.executablePath = target.executablePath.parent_path() /
                            modeDirectory / target.executablePath.filename();

    if (builder.prepareBuildDirectory(target.objectsDirectory) != 0) {
      return BUILD_ERROR_EXIT_CODE;
    }

    if (builder.prepareBuildDirectory(target.executablePath.parent_path()) !=
        0) {
      return BUILD_ERROR_EXIT_CODE;
    }

    if (builder.createBuildPlan(target, jobCount, buildMode) != 0) {
      return BUILD_ERROR_EXIT_CODE;
    }
  }

  return SUCCESS_EXIT_CODE;
}

int main(int argc, char *argv[]) {
  Builder builder;
  ConfigParser configParser;

  if (argc < 2) {
    std::cerr << "Error: no command provided.\n";
    std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
    return INVALID_COMMAND_EXIT_CODE;
  }

  const std::string command = argv[1];

  if (command == "help") {
    printHelp(argv[0]);
    return SUCCESS_EXIT_CODE;
  }

  if (command == "build") {
    if (argc != 3 && argc != 5 && argc != 7) {
      std::cerr << "Usage: " << argv[0]
                << " build <target> [--jobs <number>] "
                   "[--mode <debug|release>]\n";
      return INVALID_COMMAND_EXIT_CODE;
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
          return INVALID_COMMAND_EXIT_CODE;
        }

        if (jobCount < MINIMUM_JOB_COUNT || jobCount > MAXIMUM_JOB_COUNT) {
          std::cerr << "Error: jobs must be between " << MINIMUM_JOB_COUNT
                    << " and " << MAXIMUM_JOB_COUNT << ".\n";
          return INVALID_COMMAND_EXIT_CODE;
        }
      } else if (option == "--mode") {
        if (optionValue == "debug") {
          buildMode = BuildMode::Debug;
        } else if (optionValue == "release") {
          buildMode = BuildMode::Release;
        } else {
          std::cerr << "Error: mode must be 'debug' or 'release'.\n";
          return INVALID_COMMAND_EXIT_CODE;
        }
      } else {
        std::cerr << "Error: expected '--jobs' or '--mode'.\n";
        return INVALID_COMMAND_EXIT_CODE;
      }
    }

    std::cout << "Build mode: "
              << (buildMode == BuildMode::Debug ? "debug" : "release") << '\n';

    const auto parsedTargets = configParser.parseTargets(CONFIG_PATH);
    if (!parsedTargets.has_value()) {
      return CONFIGURATION_ERROR_EXIT_CODE;
    }

    const std::string requestedTarget = argv[2];
    const auto &targetMap = parsedTargets.value();

    DependencyGraph dependencyGraph(targetMap);
    const auto buildOrder = dependencyGraph.createBuildOrder(requestedTarget);
    if (!buildOrder) {
      return CONFIGURATION_ERROR_EXIT_CODE;
    }

    return runBuildCommand(builder, targetMap, buildOrder.value(), jobCount,
                           buildMode);
  }

  if (command == "clean") {
    if (argc != 2) {
      std::cerr << "Error: clean does not accept additional arguments.\n";
      return INVALID_COMMAND_EXIT_CODE;
    }
    if (builder.clean() != 0) {
      return BUILD_ERROR_EXIT_CODE;
    }
    return SUCCESS_EXIT_CODE;
  }

  if (command == "list") {
    if (argc != 2) {
      std::cerr << "Error: list does not accept additional arguments.\n";
      return INVALID_COMMAND_EXIT_CODE;
    }
    const auto targets = configParser.parseTargets(CONFIG_PATH);
    return runListCommand(targets);
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return INVALID_COMMAND_EXIT_CODE;
}
