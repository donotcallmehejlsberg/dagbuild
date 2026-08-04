#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "Builder.hpp"
#include "ConfigParser.hpp"

void printHelp(const char *programName);

int runListCommand(
    const std::optional<std::unordered_map<std::string, BuildTarget>> &targets);

int runBuildCommand(Builder &builder, const ConfigParser &configParser,
                    const std::string &requestedTarget);

int main(int argc, char *argv[]) {
  Builder builder;
  ConfigParser configParser;

  if (argc < 2) {
    std::cerr << "Error: no command provided.\n";
    std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
    return 1;
  }

  const std::string command = argv[1];

  if (command == "help") {
    printHelp(argv[0]);
    return 0;
  }

  if (command == "build") {
    if (argc != 3) {
      std::cerr << "Error: expected a target name.\n";
      std::cerr << "Usage: " << argv[0] << " build <target>\n";
      return 1;
    }
    const std::string requestedTarget = argv[2];
    return runBuildCommand(builder, configParser, requestedTarget);
  }

  if (command == "clean") {
    if (argc != 2) {
      std::cerr << "Error: clean does not accept additional arguments.\n";
      return 1;
    }
    return builder.clean();
  }

  if (command == "list") {
    if (argc != 2) {
      std::cerr << "Error: list does not accept additional arguments.\n";
      return 1;
    }
    const auto targets = configParser.parseTargets("dagbuild.conf");
    return runListCommand(targets);
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}

void printHelp(const char *programName) {
  std::cout << "DAGBuild - a minimal C++ build system\n\n";
  std::cout << "Usage:\n";
  std::cout << "  " << programName << " build <target>\n";
  std::cout << "  " << programName << " clean\n";
  std::cout << "  " << programName << " list\n";
  std::cout << "  " << programName << " help\n";
}

int runListCommand(
    const std::optional<std::unordered_map<std::string, BuildTarget>>
        &targets) {
  if (!targets.has_value()) {
    return 1;
  }

  std::cout << "Available targets:\n";

  for (const auto &entry : targets.value()) {
    std::cout << "  " << entry.first << '\n';
  }

  return 0;
}

int runBuildCommand(Builder &builder, const ConfigParser &configParser,
                    const std::string &requestedTarget) {
  const std::optional<std::unordered_map<std::string, BuildTarget>> targets =
      configParser.parseTargets("dagbuild.conf");
  if (!targets.has_value()) {
    return 1;
  }

  const auto &targetMap = targets.value();
  const auto targetIterator = targetMap.find(requestedTarget);
  if (targetIterator == targetMap.end()) {
    std::cerr << "Error: target '" << requestedTarget << "' not found.\n";
    return 1;
  }

  const BuildTarget &target = targetIterator->second;
  if (builder.prepareBuildDirectory(target.objectsDirectory) != 0) {
    return 1;
  }

  return builder.createBuildPlan(target);
}
