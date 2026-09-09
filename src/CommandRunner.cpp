#include "CommandRunner.hpp"

#include <iostream>

#include "Builder.hpp"
#include "ExitCodes.hpp"

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

int CommandRunner::parseBuildMode(const std::string &optionValue,
                                  BuildMode &buildMode) {
  if (optionValue == "debug") {
    buildMode = BuildMode::Debug;
    return ExitCode::SUCCESS;
  }

  if (optionValue == "release") {
    buildMode = BuildMode::Release;
    return ExitCode::SUCCESS;
  }

  std::cerr << "Error: mode must be 'debug' or 'release'.\n";
  return ExitCode::INVALID_COMMAND;
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
