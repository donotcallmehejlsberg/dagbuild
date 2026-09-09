#include "ConfigParser.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

std::optional<std::unordered_map<std::string, BuildTarget>>
ConfigParser::parseTargets(const std::filesystem::path &configPath) const {
  std::ifstream file(configPath);
  if (!file.is_open()) {
    std::cerr << "Error: could not open configuration file: "
              << configPath.string() << '\n';
    return std::nullopt;
  }

  std::unordered_map<std::string, BuildTarget> targets;
  BuildTarget currentTarget{};
  bool readingTarget = false;
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
      if (!handleTargetStart(lineStream, targets, currentTarget,
                             readingTarget)) {
        return std::nullopt;
      }
      continue;
    }

    if (keyword == "end") {
      if (!handleTargetEnd(targets, currentTarget, readingTarget)) {
        return std::nullopt;
      }
      continue;
    }

    if (!readingTarget) {
      std::cerr << "Error: keyword outside a target: " << keyword << '\n';
      return std::nullopt;
    }

    if (!handleTargetProperty(keyword, lineStream, currentTarget)) {
      return std::nullopt;
    }
  }

  if (readingTarget) {
    std::cerr << "Error: missing 'end' for target '" << currentTarget.name
              << "'.\n";
    return std::nullopt;
  }

  if (!validateDependencies(targets)) {
    return std::nullopt;
  }

  return targets;
}

bool ConfigParser::handleTargetStart(
    std::istringstream &lineStream,
    const std::unordered_map<std::string, BuildTarget> &targets,
    BuildTarget &currentTarget, bool &readingTarget) const {
  if (readingTarget) {
    std::cerr << "Error: missing 'end' for target '" << currentTarget.name
              << "'.\n";
    return false;
  }

  std::string targetName;
  if (!(lineStream >> targetName)) {
    std::cerr << "Error: target name is missing.\n";
    return false;
  }

  if (targets.contains(targetName)) {
    std::cerr << "Error: duplicate target name: " << targetName << '\n';
    return false;
  }

  currentTarget = BuildTarget{};
  currentTarget.name = targetName;
  readingTarget = true;
  return true;
}

bool ConfigParser::handleTargetEnd(
    std::unordered_map<std::string, BuildTarget> &targets,
    BuildTarget &currentTarget, bool &readingTarget) const {
  if (!readingTarget) {
    std::cerr << "Error: unexpected 'end' outside a target.\n";
    return false;
  }

  if (!validateTarget(currentTarget)) {
    return false;
  }

  targets.emplace(currentTarget.name, currentTarget);
  currentTarget = BuildTarget{};
  readingTarget = false;
  return true;
}

bool ConfigParser::handleTargetProperty(const std::string &keyword,
                                       std::istringstream &lineStream,
                                       BuildTarget &currentTarget) const {
  if (keyword == "sources") {
    return parseSources(lineStream, currentTarget);
  }

  if (keyword == "headers") {
    parseHeaders(lineStream, currentTarget);
    return true;
  }

  if (keyword == "depends") {
    parseDependencies(lineStream, currentTarget);
    return true;
  }

  if (keyword == "objects") {
    return parseObjectsDirectory(lineStream, currentTarget);
  }

  if (keyword == "output") {
    return parseOutputPath(lineStream, currentTarget);
  }

  std::cerr << "Error: unknown configuration keyword: " << keyword << '\n';
  return false;
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

void ConfigParser::parseDependencies(std::istringstream &lineStream,
                                     BuildTarget &target) const {
  std::string pathText;

  while (lineStream >> pathText) {
    target.dependencyNames.push_back(pathText);
  }
}

// targets
// ├── "core"    → BuildTarget core
// ├── "network" → BuildTarget network
// └── "app"     → BuildTarget app"
// BuildTarget app: dependencyNames = ["core", "ghost"]

bool ConfigParser::validateDependencies(
    const std::unordered_map<std::string, BuildTarget> &targets) const {
  for (const auto &t : targets) {
    const std::string &targetName = t.first;
    const BuildTarget &target = t.second;

    for (const auto &dependencyName : target.dependencyNames) {
      if (!targets.contains(dependencyName)) {
        std::cerr << "Error: target '" << targetName
                  << "' depends on unknown target '" << dependencyName
                  << "'.\n";
        return false;
      }
    }
  }
  return true;
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
