#include <gtest/gtest.h>

#include "BuildTarget.hpp"
#include "DependencyGraph.hpp"

TEST(DependencyGraphTest, CreatesBuildOrderForLinearDependencies) {
  BuildTarget utils;
  utils.dependencyNames = {};

  BuildTarget core;
  core.dependencyNames = {"utils"};

  BuildTarget app;
  app.dependencyNames = {"core"};

  app.name = "app";
  utils.name = "utils";
  core.name = "core";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(utils.name, utils);
  targets.emplace(core.name, core);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_TRUE(buildOrder.has_value());

  const std::vector<std::string> expectedOrder{"utils", "core", "app"};

  EXPECT_EQ(expectedOrder, buildOrder.value());
}

TEST(DependencyGraphTest, RejectsDependencyCycle) {
  BuildTarget core;
  core.dependencyNames = {"app"};

  BuildTarget app;
  app.dependencyNames = {"core"};

  app.name = "app";
  core.name = "core";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(core.name, core);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_FALSE(buildOrder.has_value());
}

TEST(DependencyGraphTest, RejectsUnknownRequestedTarget) {
  BuildTarget utils;
  utils.dependencyNames = {};

  BuildTarget core;
  core.dependencyNames = {"utils"};

  BuildTarget app;
  app.dependencyNames = {"core"};

  app.name = "app";
  utils.name = "utils";
  core.name = "core";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(utils.name, utils);
  targets.emplace(core.name, core);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("Gta6");
  ASSERT_FALSE(buildOrder.has_value());
}

TEST(DependencyGraphTest, AddsSharedDependencyOnlyOnce) {
  BuildTarget utils;
  utils.dependencyNames = {};

  BuildTarget core;
  core.dependencyNames = {"utils"};

  BuildTarget app;
  app.dependencyNames = {"core", "utils"};

  app.name = "app";
  utils.name = "utils";
  core.name = "core";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(utils.name, utils);
  targets.emplace(core.name, core);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_TRUE(buildOrder.has_value());

  const std::vector<std::string> expectedOrder{"utils", "core", "app"};

  EXPECT_EQ(expectedOrder, buildOrder.value());
}

TEST(DependencyGraphTest, RejectsIndirectDependencyCycle) {
  BuildTarget utils;
  utils.dependencyNames = {"app"};

  BuildTarget core;
  core.dependencyNames = {"utils"};

  BuildTarget data;
  data.dependencyNames = {"core"};

  BuildTarget app;
  app.dependencyNames = {"data"};

  app.name = "app";
  utils.name = "utils";
  core.name = "core";
  data.name = "data";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(utils.name, utils);
  targets.emplace(core.name, core);
  targets.emplace(data.name, data);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_FALSE(buildOrder.has_value());
}

TEST(DependencyGraphTest, RejectsSelfDependency) {
  BuildTarget utils;
  utils.dependencyNames = {"utils"};

  BuildTarget core;
  core.dependencyNames = {"utils"};

  BuildTarget data;
  data.dependencyNames = {"core"};

  BuildTarget app;
  app.dependencyNames = {"data"};

  app.name = "app";
  utils.name = "utils";
  core.name = "core";
  data.name = "data";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(utils.name, utils);
  targets.emplace(core.name, core);
  targets.emplace(data.name, data);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_FALSE(buildOrder.has_value());
}

TEST(DependencyGraphTest, RejectsMissingDependency) {
  BuildTarget core;
  core.dependencyNames = {};

  BuildTarget data;
  data.dependencyNames = {"core"};

  BuildTarget app;
  app.dependencyNames = {"data", "ghost"};

  app.name = "app";
  core.name = "core";
  data.name = "data";

  std::unordered_map<std::string, BuildTarget> targets;
  targets.emplace(app.name, app);
  targets.emplace(core.name, core);
  targets.emplace(data.name, data);

  DependencyGraph dependencyGraph(targets);
  const auto buildOrder = dependencyGraph.createBuildOrder("app");
  ASSERT_FALSE(buildOrder.has_value());
}
