#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <filesystem>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "BuildTarget.hpp"

class ConfigParser {
 public:
  std::optional<std::unordered_map<std::string, BuildTarget>> parseTargets(
      const std::filesystem::path &configPath) const;

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
