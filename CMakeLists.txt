cmake_minimum_required(VERSION 3.25)
project(ecs)

option(TINYECS_TESTS "Build tinyecs tests" OFF)

add_library(ecs INTERFACE)
target_include_directories(ecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET ecs PROPERTY CXX_STANDARD 20)

if(TINYECS_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

add_executable(example example.cpp)
target_link_libraries(example PRIVATE ecs)
set_property(TARGET example PROPERTY CXX_STANDARD 20)
