include(FetchContent)

FetchContent_Declare(
    benchmark
    SYSTEM EXCLUDE_FROM_ALL
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.5
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(benchmark)
target_compile_options(benchmark PRIVATE -w)

add_executable(tinyecs_benchmarks tinyecs.cpp)
init_target(tinyecs_benchmarks)
target_link_libraries(tinyecs_benchmarks PRIVATE tinyecs benchmark::benchmark benchmark::benchmark_main)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(tinyecs_benchmarks PRIVATE -Wno-global-constructors)
endif()

if(TINYECS_THIRD_PARTY_BENCHMARKS)
    FetchContent_Declare(
        EnTT
        SYSTEM EXCLUDE_FROM_ALL
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
    )
    FetchContent_MakeAvailable(EnTT)

    FetchContent_Declare(
        flecs
        SYSTEM EXCLUDE_FROM_ALL
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v4.1.5
    )

    set(FLECS_SHARED OFF CACHE BOOL "" FORCE)
    set(FLECS_PIC OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(flecs)

    add_executable(tinyecs_benchmarks_entt entt.cpp)
    init_target(tinyecs_benchmarks_entt)
    target_link_libraries(tinyecs_benchmarks_entt PRIVATE EnTT benchmark::benchmark benchmark::benchmark_main)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	    target_compile_options(tinyecs_benchmarks_entt PRIVATE -Wno-global-constructors)
    endif()

    add_executable(tinyecs_benchmarks_flecs flecs.cpp)
    init_target(tinyecs_benchmarks_flecs)
    target_link_libraries(tinyecs_benchmarks_flecs PRIVATE flecs_static benchmark::benchmark benchmark::benchmark_main)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	    target_compile_options(tinyecs_benchmarks_flecs PRIVATE -Wno-global-constructors)
    endif()
endif()