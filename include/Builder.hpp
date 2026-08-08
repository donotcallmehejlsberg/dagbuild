#ifndef BUILDER_HPP
#define BUILDER_HPP

#include <filesystem>
#include <vector>

#include "BuildMode.hpp"
#include "BuildTarget.hpp"

class Builder {
 public:
  int clean();

  int prepareBuildDirectory(const std::filesystem::path &objectsDirectory);

  int createBuildPlan(const BuildTarget &target, int jobCount,
                      BuildMode buildMode);

 private:
  int compileSource(const std::filesystem::path &sourcePath,
                    const std::filesystem::path &objectPath,
                    BuildMode buildMode);

  int linkExecutable(const std::vector<std::filesystem::path> &objectPaths,
                     const std::filesystem::path &executablePath);

  bool needsCompilation(const std::filesystem::path &sourcePath,
                        const std::filesystem::path &objectPath,
                        const std::vector<std::filesystem::path> &headerPaths);

  bool isValidSourceFile(const std::filesystem::path &sourcePath);

  bool compileSourcesIfNeeded(
      const BuildTarget &target,
      const std::vector<std::filesystem::path> &objectPaths, int jobCount,
      BuildMode buildMode);

  bool needsLinking(const std::vector<std::filesystem::path> &objectPaths,
                    const std::filesystem::path &executablePath);
};

#endif  // BUILDER_HPP
