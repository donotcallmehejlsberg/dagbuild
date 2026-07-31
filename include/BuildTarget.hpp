#ifndef BUILD_TARGET_HPP
#define BUILD_TARGET_HPP

#include <filesystem>
#include <string>
#include <vector>

struct BuildTarget {
  std::string name;
  std::vector<std::filesystem::path> sourcePaths;
  std::vector<std::filesystem::path> headerPaths;
  std::filesystem::path objectsDirectory;
  std::filesystem::path executablePath;
};

#endif // BUILD_TARGET_HPP
