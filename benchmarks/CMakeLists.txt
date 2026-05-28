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

add_executable(tinyecs_benchmarks tinyecs.cpp)
target_link_libraries(tinyecs_benchmarks PRIVATE tinyecs benchmark::benchmark benchmark::benchmark_main)
set_property(TARGET tinyecs_benchmarks PROPERTY CXX_STANDARD 20)

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
    target_link_libraries(tinyecs_benchmarks_entt PRIVATE EnTT benchmark::benchmark benchmark::benchmark_main)
    set_property(TARGET tinyecs_benchmarks_entt PROPERTY CXX_STANDARD 20)

    add_executable(tinyecs_benchmarks_flecs flecs.cpp)
    target_link_libraries(tinyecs_benchmarks_flecs PRIVATE flecs_static benchmark::benchmark benchmark::benchmark_main)
    set_property(TARGET tinyecs_benchmarks_flecs PROPERTY CXX_STANDARD 20)
endif()