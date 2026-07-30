#include <cstdint>
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Error: expected exactly one command.\n";
    std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
    return 1;
  }

  std::string command = argv[1];

  if (command == "help") {
    std::cout << "DAGBuild - a minimal C++ build system\n\n";
    std::cout << "Usage:\n";
    std::cout << "  " << argv[0] << " build\n";
    std::cout << "  " << argv[0] << " clean\n";
    std::cout << "  " << argv[0] << " help\n";
    return 0;
  }

  if (command == "build") {
    return prepareBuildDirectory();
  }

  if (command == "clean") {
    return clean();
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}
