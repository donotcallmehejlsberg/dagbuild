#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "BuildTarget.hpp"
#include "Builder.hpp"

int main(int argc, char *argv[]) {
  Builder builder;

  if (argc != 2) {
    std::cerr << "Error: expected exactly one command.\n";
    std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
    return 1;
  }

  const std::string command = argv[1];

  if (command == "help") {
    std::cout << "DAGBuild - a minimal C++ build system\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << argv[0] << " build\n";
    std::cout << "  " << argv[0] << " clean\n";
    std::cout << "  " << argv[0] << " help\n";
    return 0;
  }

  if (command == "build") {
    if (builder.prepareBuildDirectory() != 0) {
      return 1;
    }

    const BuildTarget helloTarget{
        "hello",
        {"examples/hello/main.cpp", "examples/hello/Greeter.cpp"},
        {"examples/hello/Greeter.hpp"},
        ".dagbuild/objects",
        ".dagbuild/hello"};

    return builder.createBuildPlan(helloTarget);
  }

  if (command == "clean") {
    return builder.clean();
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}
