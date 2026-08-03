#include "ConfigParser.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

std::optional<BuildTarget> ConfigParser::parseTarget(
    const std::filesystem::path &configPath,
    const std::string &requestedTarget) const {
  std::ifstream file(configPath);

  if (!file.is_open()) {
    std::cerr << "Error: could not open configuration file: "
              << configPath.string() << '\n';
    return std::nullopt;
  }

  BuildTarget currentTarget{};
  bool readingRequestedTarget = false;
  std::string readLine;

  while (std::getline(file, readLine)) {
    std::istringstream lineStream(readLine);
    std::string keyword;

    if (!(lineStream >> keyword)) {
      continue;
    }

    if (keyword.starts_with('#')) {
      continue;
    }

    if (keyword == "target") {
      std::string targetName;

      if (!(lineStream >> targetName)) {
        std::cerr << "Error: target name is missing.\n";
        return std::nullopt;
      }

      if (readingRequestedTarget) {
        std::cerr << "Error: missing 'end' for target '" << requestedTarget
                  << "'.\n";
        return std::nullopt;
      }

      readingRequestedTarget = targetName == requestedTarget;

      if (readingRequestedTarget) {
        currentTarget = BuildTarget{};
        currentTarget.name = targetName;
      }

      continue;
    }

    if (keyword == "end") {
      if (readingRequestedTarget) {
        if (!validateTarget(currentTarget)) {
          return std::nullopt;
        }

        return currentTarget;
      }

      continue;
    }

    if (!readingRequestedTarget) {
      continue;
    }

    if (keyword == "sources") {
      if (!parseSources(lineStream, currentTarget)) {
        return std::nullopt;
      }

      continue;
    }

    if (keyword == "headers") {
      parseHeaders(lineStream, currentTarget);
      continue;
    }

    if (keyword == "objects") {
      if (!parseObjectsDirectory(lineStream, currentTarget)) {
        return std::nullopt;
      }

      continue;
    }

    if (keyword == "output") {
      if (!parseOutputPath(lineStream, currentTarget)) {
        return std::nullopt;
      }

      continue;
    }

    std::cerr << "Error: unknown configuration keyword: " << keyword << '\n';
    return std::nullopt;
  }

  if (readingRequestedTarget) {
    std::cerr << "Error: missing 'end' for target '" << requestedTarget
              << "'.\n";
  } else {
    std::cerr << "Error: target '" << requestedTarget << "' not found.\n";
  }

  return std::nullopt;
}

std::optional<std::vector<std::string>> ConfigParser::parseTargetNames(
    const std::filesystem::path &configPath) const {
  std::ifstream file(configPath);
  if (!file.is_open()) {
    std::cerr << "Error: could not open configuration file: "
              << configPath.string() << '\n';
    return std::nullopt;
  }

  std::vector<std::string> targetNames;
  std::string readLine;

  while (std::getline(file, readLine)) {
    std::istringstream lineStream(readLine);

    std::string keyword;

    if (!(lineStream >> keyword)) {
      continue;
    }

    if (keyword.starts_with('#')) {
      continue;
    }

    if (keyword == "target") {
      std::string targetName;

      if (!(lineStream >> targetName)) {
        std::cerr << "Error: target name is missing.\n";
        return std::nullopt;
      }
      targetNames.push_back(targetName);
    }
  }
  return targetNames;
}

bool ConfigParser::parseSources(std::istringstream &lineStream,
                                BuildTarget &target) const {
  std::string pathText;

  while (lineStream >> pathText) {
    target.sourcePaths.push_back(pathText);
  }

  if (target.sourcePaths.empty()) {
    std::cerr << "Error: sources list is empty.\n";
    return false;
  }

  return true;
}

void ConfigParser::parseHeaders(std::istringstream &lineStream,
                                BuildTarget &target) const {
  std::string pathText;

  while (lineStream >> pathText) {
    target.headerPaths.push_back(pathText);
  }
}

bool ConfigParser::parseObjectsDirectory(std::istringstream &lineStream,
                                         BuildTarget &target) const {
  std::string pathText;

  if (!(lineStream >> pathText)) {
    std::cerr << "Error: objects path is missing.\n";
    return false;
  }

  target.objectsDirectory = pathText;
  return true;
}

bool ConfigParser::parseOutputPath(std::istringstream &lineStream,
                                   BuildTarget &target) const {
  std::string pathText;

  if (!(lineStream >> pathText)) {
    std::cerr << "Error: output path is missing.\n";
    return false;
  }

  target.executablePath = pathText;
  return true;
}

bool ConfigParser::validateTarget(const BuildTarget &target) const {
  if (target.sourcePaths.empty()) {
    std::cerr << "Error: target has no source files.\n";
    return false;
  }

  if (target.objectsDirectory.empty()) {
    std::cerr << "Error: objects path is missing.\n";
    return false;
  }

  if (target.executablePath.empty()) {
    std::cerr << "Error: output path is missing.\n";
    return false;
  }

  return true;
}
