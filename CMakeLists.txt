cmake_minimum_required(VERSION 3.25)
project(ecs)

include(tinyecs.cmake)

string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}" TINYECS_TOP_LEVEL)

option(TINYECS_SAMPLES "Build tinyecs samples" ${TINYECS_TOP_LEVEL})
option(TINYECS_TESTS "Build tinyecs tests" ${TINYECS_TOP_LEVEL})
option(TINYECS_BENCHMARKS "Build tinyecs benchmarks" ${TINYECS_TOP_LEVEL})
option(TINYECS_THIRD_PARTY_BENCHMARKS "Build entt and flecs benchmarks" OFF)

add_library(tinyecs INTERFACE)
target_include_directories(tinyecs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
set_property(TARGET tinyecs PROPERTY CXX_STANDARD 20)

if(TINYECS_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()

if(TINYECS_SAMPLES)
  add_subdirectory(samples)
endif()

if(TINYECS_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
