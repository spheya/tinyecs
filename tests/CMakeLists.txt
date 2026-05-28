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
target_compile_options(gtest PRIVATE -w)

file(GLOB_RECURSE TEST_SOURCES *.cpp)
add_executable(tinyecs_tests ${TEST_SOURCES})
init_target(tinyecs_tests)
target_link_libraries(tinyecs_tests PRIVATE tinyecs GTest::gtest_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(tinyecs_tests PRIVATE -Wno-global-constructors)
endif()
gtest_discover_tests(tinyecs_tests)