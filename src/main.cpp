#include <iostream>
#include <string>

#include "Builder.hpp"
#include "CommandRunner.hpp"
#include "ConfigParser.hpp"
#include "ExitCodes.hpp"

namespace {

constexpr char CONFIG_PATH[] = "dagbuild.conf";

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
    return commandRunner.runBuildCommand(builder, configParser, CONFIG_PATH,
                                         argc, argv);
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
