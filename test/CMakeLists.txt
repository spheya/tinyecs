set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GoogleTest)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES *.cpp)
add_executable(ecs_tests ${TEST_SOURCES})
target_link_libraries(ecs_tests PRIVATE ecs GTest::gtest_main)
gtest_discover_tests(ecs_tests)
