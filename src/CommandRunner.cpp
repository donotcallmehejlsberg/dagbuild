#include "CommandRunner.hpp"

#include <exception>
#include <iostream>

#include "Builder.hpp"
#include "DependencyGraph.hpp"
#include "ExitCodes.hpp"

namespace {

constexpr int DEFAULT_JOB_COUNT = 1;
constexpr int MINIMUM_JOB_COUNT = 1;
constexpr int MAXIMUM_JOB_COUNT = 10;

}  // namespace

void CommandRunner::runPrintHelp(const char *programName) {
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

std::optional<int> CommandRunner::parseJobCount(
    const std::string &optionValue) const {
  int jobCount;
  try {
    jobCount = std::stoi(optionValue);
  } catch (const std::exception &) {
    std::cerr << "Error: jobs must be a number.\n";
    return std::nullopt;
  }

  if (jobCount < MINIMUM_JOB_COUNT || jobCount > MAXIMUM_JOB_COUNT) {
    std::cerr << "Error: jobs must be between " << MINIMUM_JOB_COUNT << " and "
              << MAXIMUM_JOB_COUNT << ".\n";
    return std::nullopt;
  }

  return jobCount;
}

std::optional<BuildMode> CommandRunner::parseBuildMode(
    const std::string &optionValue) const {
  if (optionValue == "debug") {
    return BuildMode::Debug;
  }

  if (optionValue == "release") {
    return BuildMode::Release;
  }

  std::cerr << "Error: mode must be 'debug' or 'release'.\n";
  return std::nullopt;
}

std::optional<BuildOptions> CommandRunner::parseBuildOptions(
    int argc, char *argv[]) const {
  BuildOptions buildOptions{DEFAULT_JOB_COUNT, BuildMode::Debug};

  for (int i = 3; i < argc; i += 2) {
    const std::string option = argv[i];
    const std::string optionValue = argv[i + 1];

    if (option == "--jobs") {
      const auto jobCount = parseJobCount(optionValue);
      if (!jobCount.has_value()) {
        return std::nullopt;
      }
      buildOptions.jobCount = jobCount.value();
    } else if (option == "--mode") {
      const auto buildMode = parseBuildMode(optionValue);
      if (!buildMode.has_value()) {
        return std::nullopt;
      }
      buildOptions.buildMode = buildMode.value();
    } else {
      std::cerr << "Error: expected '--jobs' or '--mode'.\n";
      return std::nullopt;
    }
  }

  return buildOptions;
}

int CommandRunner::runBuildCommand(
    Builder &builder, const ConfigParser &configParser,
    const std::filesystem::path &configPath, int argc, char *argv[]) {
  const auto buildOptions = parseBuildOptions(argc, argv);
  if (!buildOptions.has_value()) {
    return ExitCode::INVALID_COMMAND;
  }

  const int jobCount = buildOptions->jobCount;
  const BuildMode buildMode = buildOptions->buildMode;

  std::cout << "Build mode: "
            << (buildMode == BuildMode::Debug ? "debug" : "release") << '\n';

  const auto parsedTargets = configParser.parseTargets(configPath);
  if (!parsedTargets.has_value()) {
    return ExitCode::CONFIGURATION_ERROR;
  }

  const std::string requestedTarget = argv[2];
  const auto &targetMap = parsedTargets.value();

  DependencyGraph dependencyGraph(targetMap);
  const auto buildOrder = dependencyGraph.createBuildOrder(requestedTarget);
  if (!buildOrder.has_value()) {
    return ExitCode::CONFIGURATION_ERROR;
  }

  return runBuild(builder, targetMap, buildOrder.value(), jobCount, buildMode);
}

int CommandRunner::runListCommand(
    const std::optional<std::unordered_map<std::string, BuildTarget>>
        &targets) {
  if (!targets.has_value()) {
    return ExitCode::CONFIGURATION_ERROR;
  }

  std::cout << "Available targets:\n";
  for (const auto &entry : targets.value()) {
    std::cout << "  " << entry.first << '\n';
  }

  return ExitCode::SUCCESS;
}

int CommandRunner::runBuild(
    Builder &builder,
    const std::unordered_map<std::string, BuildTarget> &targetMap,
    const std::vector<std::string> &buildOrder, int jobCount,
    BuildMode buildMode) {
  for (const std::string &targetName : buildOrder) {
    const auto targetIterator = targetMap.find(targetName);

    if (targetIterator == targetMap.end()) {
      std::cerr << "Error: target '" << targetName << "' not found.\n";
      return ExitCode::CONFIGURATION_ERROR;
    }

    BuildTarget target = targetIterator->second;
    const std::string modeDirectory =
        buildMode == BuildMode::Debug ? "debug" : "release";
    target.objectsDirectory /= modeDirectory;

    target.executablePath = target.executablePath.parent_path() /
                            modeDirectory / target.executablePath.filename();

    if (builder.prepareBuildDirectory(target.objectsDirectory) != 0) {
      return ExitCode::BUILD_ERROR;
    }

    if (builder.prepareBuildDirectory(target.executablePath.parent_path()) !=
        0) {
      return ExitCode::BUILD_ERROR;
    }

    if (builder.createBuildPlan(target, jobCount, buildMode) != 0) {
      return ExitCode::BUILD_ERROR;
    }
  }

  return ExitCode::SUCCESS;
}

int CommandRunner::runClean(Builder &builder) {
  if (builder.clean() != 0) {
    return ExitCode::BUILD_ERROR;
  }
  return ExitCode::SUCCESS;
}
