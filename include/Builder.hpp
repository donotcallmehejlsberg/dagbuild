#ifndef BUILDER_HPP
#define BUILDER_HPP

#include "BuildTarget.hpp"

class Builder {
 public:
  int clean();

  int prepareBuildDirectory(const std::filesystem::path &objectsDirectory);

  int compileSource(const std::filesystem::path &sourcePath,
                    const std::filesystem::path &objectPath);

  int linkExecutable(const std::vector<std::filesystem::path> &objectPaths,
                     const std::filesystem::path &executablePath);

  bool needsCompilation(const std::filesystem::path &sourcePath,
                        const std::filesystem::path &objectPath,
                        const std::vector<std::filesystem::path> &headerPaths);

  bool needsLinking(const std::vector<std::filesystem::path> &objectPaths,
                    const std::filesystem::path &executablePath);

  int createBuildPlan(const BuildTarget &target);
};

#endif  // BUILDER_HPP
