#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>

#include "Builder.hpp"
#include "ConfigParser.hpp"
#include "DependencyGraph.hpp"

void printHelp(const char *programName);

int runListCommand(
    const std::optional<std::unordered_map<std::string, BuildTarget>> &targets);

int runBuildCommand(
    Builder &builder,
    const std::unordered_map<std::string, BuildTarget> &targetMap,
    const std::vector<std::string> &buildOrder, int number_of_theards);

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
    if (argc != 3 && argc != 5) {
      std::cerr << "Usage: " << argv[0]
                << " build <target> [--jobs <number>]\n";
      return 1;
    }

    int num_of_thread = 1;
    if (argc == 5) {
      const std::string jobs = argv[3];
      const std::string threads = argv[4];
      if (jobs != "--jobs") {
        std::cerr << "Error: expected '--jobs'.\n";
        return 1;
      }

      try {
        num_of_thread = std::stoi(threads);
      } catch (const std::exception &) {
        std::cerr << "Error: jobs must be a number.\n";
        return 1;
      }

      if (num_of_thread <= 0 || num_of_thread > 10) {
        std::cerr << "Error: jobs must be between 1 and 10.\n";
        return 1;
      }
    }

    const auto parsedTargets = configParser.parseTargets("dagbuild.conf");
    if (!parsedTargets.has_value()) {
      return 1;
    }

    const std::string requestedTarget = argv[2];
    const auto &targetMap = parsedTargets.value();

    DependencyGraph dependencyGraph(targetMap);
    const auto buildOrder = dependencyGraph.createBuildOrder(requestedTarget);
    if (!buildOrder) {
      return 1;
    }

    return runBuildCommand(builder, targetMap, buildOrder.value(), num_of_thread);
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

int runBuildCommand(
    Builder &builder,
    const std::unordered_map<std::string, BuildTarget> &targetMap,
    const std::vector<std::string> &buildOrder, int number_of_theards) {
  for (const std::string &targetName : buildOrder) {
    const auto targetIterator = targetMap.find(targetName);

    if (targetIterator == targetMap.end()) {
      std::cerr << "Error: target '" << targetName << "' not found.\n";
      return 1;
    }

    const BuildTarget &target = targetIterator->second;

    if (builder.prepareBuildDirectory(target.objectsDirectory) != 0) {
      return 1;
    }

    if (builder.createBuildPlan(target, number_of_theards) != 0) {
      return 1;
    }
  }

  return 0;
}
