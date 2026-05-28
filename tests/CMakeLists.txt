set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GoogleTest)

include(FetchContent)
FetchContent_Declare(
  googletest
  SYSTEM EXCLUDE_FROM_ALL
  URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES *.cpp)
add_executable(tinyecs_tests ${TEST_SOURCES})
target_link_libraries(tinyecs_tests PRIVATE tinyecs GTest::gtest_main)
set_property(TARGET tinyecs_tests PROPERTY CXX_STANDARD 20)
gtest_discover_tests(tinyecs_tests)