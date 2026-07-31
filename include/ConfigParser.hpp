#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <filesystem>
#include <optional>
#include <sstream>
#include <string>

#include "BuildTarget.hpp"

class ConfigParser {
 public:
  std::optional<BuildTarget> parseTarget(
      const std::filesystem::path &configPath,
      const std::string &requestedTarget) const;

 private:
  bool parseSources(std::istringstream &lineStream, BuildTarget &target) const;

  void parseHeaders(std::istringstream &lineStream, BuildTarget &target) const;

  bool parseObjectsDirectory(std::istringstream &lineStream,
                             BuildTarget &target) const;

  bool parseOutputPath(std::istringstream &lineStream,
                       BuildTarget &target) const;

  bool validateTarget(const BuildTarget &target) const;
};

#endif  // CONFIG_PARSER_HPP