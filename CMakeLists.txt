cmake_minimum_required(VERSION 3.20)

project(
  DAGBuild
  VERSION 0.1.0
  LANGUAGES CXX
)

# Main DAGBuild executable
add_executable(dagbuild
  src/main.cpp
  src/Builder.cpp
  src/ConfigParser.cpp
  src/DependencyGraph.cpp
)

target_compile_features(dagbuild
  PRIVATE
  cxx_std_20
)

set_target_properties(dagbuild PROPERTIES
  CXX_EXTENSIONS OFF
)

target_include_directories(dagbuild
  PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
)

# Compiler warnings
if(MSVC)
  target_compile_options(dagbuild PRIVATE
    /W4
    /WX
    /permissive-
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang|AppleClang)$")
  target_compile_options(dagbuild PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wconversion
    -Wsign-conversion
    -Wshadow
  )
endif()

# GoogleTest dependency
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)

# Tests
enable_testing()

add_executable(dagbuild_tests
  tests/MainTests.cpp
  tests/ConfigParserTest.cpp
  src/ConfigParser.cpp
  tests/DependencyGraphTest.cpp
  src/DependencyGraph.cpp
)

target_compile_features(dagbuild_tests
  PRIVATE
  cxx_std_20
)

target_include_directories(dagbuild_tests
  PRIVATE
  "${PROJECT_SOURCE_DIR}/include"
)

set_target_properties(dagbuild_tests PROPERTIES
  CXX_EXTENSIONS OFF
)

target_link_libraries(dagbuild_tests
  PRIVATE
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(dagbuild_tests)
