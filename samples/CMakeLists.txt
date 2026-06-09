include(FetchContent)
FetchContent_Declare(
    glfw
    SYSTEM EXCLUDE_FROM_ALL
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

add_library(tinyecs_sample_common INTERFACE)
target_include_directories(tinyecs_sample_common SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tinyecs_sample_common INTERFACE glfw)

add_subdirectory(bouncing)
add_subdirectory(reflection)
