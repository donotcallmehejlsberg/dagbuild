#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct BuildTarget {
  std::string name;
  std::vector<fs::path> sourcePaths;
  std::vector<fs::path> headerPaths;
  fs::path objectsDirectory;
  fs::path executablePath;
};

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

  std::cout << "Compiling " << sourcePath.filename().string() << "...\n";

  const int result = std::system(command.c_str());
  if (result != 0) {
    std::cerr << "Error: compilation failed.\n";
    return 1;
  }

  std::cout << "Compilation completed successfully.\n";
  return 0;
}

int linkExecutable(const std::vector<fs::path> &objectPaths,
                   const fs::path &executablePath) {
  if (objectPaths.empty()) {
    std::cerr << "Error: no object files to link.\n";
    return 1;
  }

  std::string command = "c++";

  for (const fs::path &objectPath : objectPaths) {
    command += " \"" + objectPath.string() + "\"";
  }

  command += " -o \"" + executablePath.string() + "\"";

  std::cout << "Link command: " << command << '\n';

  const int result = std::system(command.c_str());
  if (result != 0) {
    std::cerr << "Error: linking failed.\n";
    return 1;
  }

  std::cout << "Build completed successfully.\n";
  return 0;
}

bool needsCompilation(const fs::path &sourcePath, const fs::path &objectPath,
                      const std::vector<fs::path> &headerPaths) {
  if (!fs::exists(objectPath)) {
    return true;
  }

  if (fs::last_write_time(sourcePath) > fs::last_write_time(objectPath)) {
    return true;
  }

  for (const fs::path &headerPath : headerPaths) {
    if (fs::last_write_time(headerPath) > fs::last_write_time(objectPath)) {
      return true;
    }
  }

  return false;
}

bool needsLinking(const std::vector<fs::path> &objectPaths,
                  const fs::path &executablePath) {
  if (!fs::exists(executablePath)) {
    return true;
  }

  for (const fs::path &objectPath : objectPaths) {
    if (fs::last_write_time(objectPath) > fs::last_write_time(executablePath)) {
      return true;
    }
  }

  return false;
}

int createBuildPlan(const BuildTarget &target) {
  std::cout << "Building target: " << target.name << '\n';

  std::vector<fs::path> objectPaths;

  try {
    for (const fs::path &sourcePath : target.sourcePaths) {
      if (!fs::exists(sourcePath)) {
        std::cerr << "Error: source file does not exist: "
                  << sourcePath.string() << '\n';

        return 1;
      }

      if (!fs::is_regular_file(sourcePath)) {
        std::cerr << "Error: source path is not a regular file: "
                  << sourcePath.string() << '\n';

        return 1;
      }

      if (sourcePath.extension() != ".cpp") {
        std::cerr << "Error: expected a .cpp source file: "
                  << sourcePath.string() << '\n';

        return 1;
      }

      fs::path objectPath = target.objectsDirectory / sourcePath.filename();
      objectPath.replace_extension(".o");
      objectPaths.push_back(objectPath);

      std::cout << "Source: " << sourcePath.string() << '\n';
      std::cout << "Object: " << objectPath.string() << '\n';
    }
  } catch (const fs::filesystem_error &error) {
    std::cerr << "Error: failed to create build plan.\n";
    std::cerr << error.what() << '\n';
    return 1;
  }

  std::cout << "Build plan created successfully.\n";

  for (std::size_t i = 0; i < target.sourcePaths.size(); ++i) {
    if (needsCompilation(target.sourcePaths[i], objectPaths[i],
                         target.headerPaths)) {
      if (compileSource(target.sourcePaths[i], objectPaths[i]) != 0) {
        return 1;
      }
    } else {
      std::cout << "Up to date: " << target.sourcePaths[i].filename().string()
                << '\n';
    }
  }

  const fs::path executablePath = ".dagbuild/hello";
  if (!needsLinking(objectPaths, executablePath)) {
    std::cout << "Executable is up to date: "
              << executablePath.filename().string() << '\n';

    std::cout << "Nothing to build.\n";
    return 0;
  }

  return linkExecutable(objectPaths, executablePath);
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

    const BuildTarget helloTarget{
        "hello",
        {"examples/hello/main.cpp", "examples/hello/Greeter.cpp"},
        {"examples/hello/Greeter.hpp"},
        ".dagbuild/objects",
        ".dagbuild/hello"};

    return createBuildPlan(helloTarget);
  }

  if (command == "clean") {
    return clean();
  }

  std::cerr << "Error: unknown command '" << command << "'.\n";
  std::cerr << "Use '" << argv[0] << " help' for usage information.\n";
  return 1;
}
