#include "Builder.hpp"

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

int Builder::clean() {
  const std::filesystem::path path = ".dagbuild";
  if (!std::filesystem::exists(path)) {
    std::cout << "Nothing to clean.\n";
    return 0;
  }

  std::cout << "Cleaning build files...\n";

  try {
    const std::uintmax_t removedCount = std::filesystem::remove_all(path);

    std::cout << "Removed elements: " << removedCount << '\n';
    std::cout << "Clean completed successfully.\n";

    return 0;
  } catch (const std::filesystem::filesystem_error &error) {
    std::cerr << "Error: failed to clean build directory.\n";
    std::cerr << error.what() << '\n';

    return 1;
  }
}

int Builder::prepareBuildDirectory(
    const std::filesystem::path &objectsDirectory) {
  std::cout << "Preparing build directory...\n";
  try {
    if (std::filesystem::exists(objectsDirectory)) {
      if (!std::filesystem::is_directory(objectsDirectory)) {
        std::cerr << "Error: build path exists but is not a directory: "
                  << objectsDirectory.string() << '\n';

        return 1;
      }

      std::cout << "Build directory already exists.\n";
      std::cout << "Build directory is ready.\n";
      return 0;
    }

    if (std::filesystem::create_directories(objectsDirectory)) {
      std::cout << "Created: " << objectsDirectory.string() << '\n';
    }

    std::cout << "Build directory is ready.\n";
    return 0;
  } catch (const std::filesystem::filesystem_error &error) {
    std::cerr << "Error: failed to prepare build directory.\n";
    std::cerr << error.what() << '\n';
    return 1;
  }
}

int Builder::compileSource(const std::filesystem::path &sourcePath,
                           const std::filesystem::path &objectPath) {
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

int Builder::linkExecutable(
    const std::vector<std::filesystem::path> &objectPaths,
    const std::filesystem::path &executablePath) {
  if (objectPaths.empty()) {
    std::cerr << "Error: no object files to link.\n";
    return 1;
  }

  std::string command = "c++";

  for (const std::filesystem::path &objectPath : objectPaths) {
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

bool Builder::needsCompilation(
    const std::filesystem::path &sourcePath,
    const std::filesystem::path &objectPath,
    const std::vector<std::filesystem::path> &headerPaths) {
  if (!std::filesystem::exists(objectPath)) {
    return true;
  }

  if (std::filesystem::last_write_time(sourcePath) >
      std::filesystem::last_write_time(objectPath)) {
    return true;
  }

  for (const std::filesystem::path &headerPath : headerPaths) {
    if (std::filesystem::last_write_time(headerPath) >
        std::filesystem::last_write_time(objectPath)) {
      return true;
    }
  }

  return false;
}

bool Builder::needsLinking(
    const std::vector<std::filesystem::path> &objectPaths,
    const std::filesystem::path &executablePath) {
  if (!std::filesystem::exists(executablePath)) {
    return true;
  }

  for (const std::filesystem::path &objectPath : objectPaths) {
    if (std::filesystem::last_write_time(objectPath) >
        std::filesystem::last_write_time(executablePath)) {
      return true;
    }
  }

  return false;
}

bool Builder::isValidSourceFile(const std::filesystem::path &sourcePath) {
  if (!std::filesystem::exists(sourcePath)) {
    std::cerr << "Error: source file does not exist: " << sourcePath.string()
              << '\n';

    return false;
  }

  if (!std::filesystem::is_regular_file(sourcePath)) {
    std::cerr << "Error: source path is not a regular file: "
              << sourcePath.string() << '\n';

    return false;
  }

  if (sourcePath.extension() != ".cpp") {
    std::cerr << "Error: expected a .cpp source file: " << sourcePath.string()
              << '\n';

    return false;
  }

  return true;
}

bool Builder::compileSourcesIfNeeded(
    const BuildTarget &target,
    const std::vector<std::filesystem::path> &objectPaths) {
  for (std::size_t i = 0; i < target.sourcePaths.size(); ++i) {
    if (needsCompilation(target.sourcePaths[i], objectPaths[i],
                         target.headerPaths)) {
      if (compileSource(target.sourcePaths[i], objectPaths[i]) != 0) {
        return false;
      }
    } else {
      std::cout << "Up to date: " << target.sourcePaths[i].filename().string()
                << '\n';
    }
  }
  return true;
}

int Builder::createBuildPlan(const BuildTarget &target) {
  std::cout << "Building target: " << target.name << '\n';

  std::vector<std::filesystem::path> objectPaths;
  try {
    for (const std::filesystem::path &sourcePath : target.sourcePaths) {
      if (!isValidSourceFile(sourcePath)) {
        return 1;
      }

      std::filesystem::path objectPath =
          target.objectsDirectory / sourcePath.filename();
      objectPath.replace_extension(".o");
      objectPaths.push_back(objectPath);

      std::cout << "Source: " << sourcePath.string() << '\n';
      std::cout << "Object: " << objectPath.string() << '\n';
    }
  } catch (const std::filesystem::filesystem_error &error) {
    std::cerr << "Error: failed to create build plan.\n";
    std::cerr << error.what() << '\n';
    return 1;
  }

  std::cout << "Build plan created successfully.\n";
  if (!compileSourcesIfNeeded(target, objectPaths)) {
    return 1;
  }

  const std::filesystem::path executablePath = target.executablePath;
  if (!needsLinking(objectPaths, executablePath)) {
    std::cout << "Executable is up to date: "
              << executablePath.filename().string() << '\n';

    std::cout << "Nothing to build.\n";
    return 0;
  }

  return linkExecutable(objectPaths, executablePath);
}
