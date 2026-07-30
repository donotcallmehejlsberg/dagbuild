#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int clean() {
  const fs::path path = ".dagbuild";
  if (!fs::exists(path)) {
    std::cout << "Nothing to clean.\n";
    return 0;
  }

  std::cout << "Cleaning build files...\n";

  try {
    const std::uintmax_t removedCount = fs::remove_all(path);

    std::cout << "Removed elements: " << removedCount << '\n';
    std::cout << "Clean completed successfully.\n";

    return 0;
  } catch (const fs::filesystem_error &error) {
    std::cerr << "Error: failed to clean build directory.\n";
    std::cerr << error.what() << '\n';

    return 1;
  }
}

int prepareBuildDirectory() {
  const fs::path buildDirectory = ".dagbuild";
  const fs::path objectsDirectory = buildDirectory / "objects";

  std::cout << "Preparing build directory...\n";
  try {
    if (fs::exists(objectsDirectory)) {
      if (!fs::is_directory(objectsDirectory)) {
        std::cerr << "Error: build path exists but is not a directory: "
                  << objectsDirectory.string() << '\n';

        return 1;
      }

      std::cout << "Build directory already exists.\n";
      std::cout << "Build directory is ready.\n";
      return 0;
    }

    if (fs::create_directories(objectsDirectory)) {
      std::cout << "Created: " << objectsDirectory.string() << '\n';
    }

    std::cout << "Build directory is ready.\n";
    return 0;
  } catch (const fs::filesystem_error &error) {
    std::cerr << "Error: failed to prepare build directory.\n";
    std::cerr << error.what() << '\n';
    return 1;
  }
}

int compileSource(const fs::path &sourcePath, const fs::path &objectPath) {
  const std::string command = "c++ -std=c++20 -Wall -Wextra -c \"" +
                              sourcePath.string() + "\" -o \"" +
                              objectPath.string() + "\"";

  std::cout << "[1/1] Compiling " << sourcePath.string() << '\n';

  const int result = std::system(command.c_str());
  if (result != 0) {
    std::cerr << "Error: compilation failed.\n";
    return 1;
  }

  std::cout << "Compilation completed successfully.\n";
  return 0;
}

int linkExecutable(const fs::path &objectPath, const fs::path &executablePath) {
  const std::string command = "c++ \"" + objectPath.string() + "\" -o \"" +
                              executablePath.string() + "\"";

  std::cout << "[2/2] Linking hello" << executablePath.string() << '\n';

  const int result = std::system(command.c_str());
  if (result != 0) {
    std::cerr << "Error: linking failed.\n";
    return 1;
  }

  std::cerr << "Build completed successfully.\n";
  return 0;
}

int createBuildPlan() {
  const fs::path sourcePath = "examples/hello/main.cpp";
  const fs::path objectsDirectory = ".dagbuild/objects";

  try {
    if (!fs::exists(sourcePath)) {
      std::cerr << "Error: source file does not exist: " << sourcePath.string()
                << '\n';
      return 1;
    }

    if (!fs::is_regular_file(sourcePath)) {
      std::cerr << "Error: source path is not a regular file: "
                << sourcePath.string() << '\n';
      return 1;
    }

    if (sourcePath.extension() != ".cpp") {
      std::cerr << "Error: expected a .cpp source file: " << sourcePath.string()
                << '\n';
      return 1;
    }
  } catch (const fs::filesystem_error &error) {
    std::cerr << "Error: failed to create build plan.\n";
    std::cerr << error.what() << '\n';
    return 1;
  }

  fs::path objectPath = objectsDirectory / sourcePath.filename();
  objectPath.replace_extension(".o");

  const fs::path executablePath = ".dagbuild/hello";

  std::cout << "Source: " << sourcePath.string() << '\n';
  std::cout << "Object: " << objectPath.string() << '\n';
  std::cout << "Build plan created successfully.\n";

  if (compileSource(sourcePath, objectPath) != 0) {
    return 1;
  }

  return linkExecutable(objectPath, executablePath);
}

int main(int argc, char *argv[]) {
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
    if (prepareBuildDirectory() != 0) {
      return 1;
    }

    return createBuildPlan();
  }

  if (command == "clean") {
    return clean();
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}
