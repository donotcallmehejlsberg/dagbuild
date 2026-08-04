#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "ConfigParser.hpp"

TEST(ParserTest, RejectsEmptyConfigPath) {
  ConfigParser configParser;

  const auto targets = configParser.parseTargets("");

  EXPECT_FALSE(targets.has_value());
}

TEST(ParserTest, RejectsDuplicateTargetName) {
  const std::filesystem::path configPath = "duplicate_targets_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources first.cpp\n"
         << "objects build/hello\n"
         << "output build/hello\n"
         << "end\n\n"
         << "target hello\n"
         << "sources second.cpp\n"
         << "objects build/hello2\n"
         << "output build/hello2\n"
         << "end\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsMissingEnd) {
  const std::filesystem::path configPath = "missing_end_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources first.cpp\n"
         << "objects build/hello\n"
         << "output build/hello\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsMissingConfigurationFile) {
  const std::filesystem::path configPath =
      "configuration_file_that_does_not_exist.conf";

  ASSERT_FALSE(std::filesystem::exists(configPath));

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());
}

TEST(ParserTest, RejectsUnexpectedEnd) {
  const std::filesystem::path configPath = "unexpected_end.conf";

  {
    std::ofstream file(configPath);
    file << "end\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsKeywordOutsideTarget) {
  const std::filesystem::path configPath = "keyword_outside_target.conf";

  {
    std::ofstream file(configPath);
    file << "sources first.cpp\n"
         << "objects build/hello\n"
         << "output build/hello\n"
         << "end\n\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsUnknownKeyword) {
  const std::filesystem::path configPath = "unknown_keyword_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources first.cpp\n"
         << "objects build/hello\n"
         << "output build/hello\n"
         << "banana value\n"
         << "end\n\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsEmptySources) {
  const std::filesystem::path configPath = "empty_sources_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources\n"
         << "objects build/hello\n"
         << "output build/hello\n"
         << "end\n\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsMissingObjectsDirectory) {
  const std::filesystem::path configPath =
      "missing_objects_directory_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources first.cpp\n"
         << "output build/hello\n"
         << "end\n\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}

TEST(ParserTest, RejectsMissingOutputPath) {
  const std::filesystem::path configPath = "missing_output_path_test.conf";

  {
    std::ofstream file(configPath);

    file << "target hello\n"
         << "sources first.cpp\n"
         << "objects build/hello\n"
         << "end\n\n";
  }

  ConfigParser configParser;
  const auto targets = configParser.parseTargets(configPath);

  EXPECT_FALSE(targets.has_value());

  std::filesystem::remove(configPath);
}
