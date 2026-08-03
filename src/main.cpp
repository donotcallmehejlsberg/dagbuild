#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "Builder.hpp"
#include "ConfigParser.hpp"

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
    std::cout << "DAGBuild - a minimal C++ build system\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << argv[0] << " build <target>\n";
    std::cout << "  " << argv[0] << " clean\n";
    std::cout << "  " << argv[0] << " list\n";
    std::cout << "  " << argv[0] << " help\n";
    return 0;
  }

  if (command == "build") {
    if (argc != 3) {
      std::cerr << "Error: expected a target name.\n";
      std::cerr << "Usage: " << argv[0] << " build <target>\n";
      return 1;
    }

    const std::string requestedTarget = argv[2];

    const std::optional<BuildTarget> target =
        configParser.parseTarget("dagbuild.conf", requestedTarget);

    if (!target) {
      return 1;
    }

    if (builder.prepareBuildDirectory(target->objectsDirectory) != 0) {
      return 1;
    }

    return builder.createBuildPlan(*target);
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

    const std::filesystem::path configPath = "dagbuild.conf";
    const std::optional<std::vector<std::string>> targetNames =
        configParser.parseTargetNames(configPath);
    if (!targetNames.has_value()) {
      return 1;
    }

    std::cout << "Available targets:\n";

    for (const std::string &targetName : targetNames.value()) {
      std::cout << "  " << targetName << '\n';
    }

    return 0;
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}
