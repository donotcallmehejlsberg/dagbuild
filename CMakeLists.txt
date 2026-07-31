cmake_minimum_required(VERSION 3.20)

project(
  DAGBuild
  VERSION 0.1.0
  LANGUAGES CXX
)

add_executable(dagbuild
  src/main.cpp
  src/Builder.cpp
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
