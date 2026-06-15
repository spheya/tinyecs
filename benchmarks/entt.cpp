#include <cstddef>
#include <cstdint>
#include <vector>

#include <benchmark/benchmark.h>
#include <entt/entt.hpp>

#include "tests/common.hpp"

template<typename... Components>
static std::vector<entt::entity> add_entities(entt::registry& registry, size_t count) {
	std::vector<entt::entity> results;
	results.reserve(count);

	for(size_t i = 0; i < count; ++i) {
		entt::entity entity = registry.create();
		((registry.emplace<Components>(entity, component_generator<Components>{}(unsigned(i)))), ...);
		results.push_back(entity);
	}
	return results;
}

template<size_t Size>
static void simple_scene_iteration(benchmark::State& state) {
	entt::registry registry;
	add_entities<small_component<0>, small_component<1>>(registry, Size);

	auto view = registry.view<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		view.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(registry);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void complex_scene_iteration(benchmark::State& state) {
	entt::registry registry;

	add_entities<small_component<0>, small_component<1>>(registry, Size / 4);
	add_entities<big_component<0>, big_component<1>>(registry, Size);
	add_entities<small_component<0>, big_component<1>>(registry, Size);
	add_entities<small_component<1>, big_component<1>>(registry, Size);
	add_entities<small_component<1>, big_component<2>>(registry, Size);
	add_entities<small_component<1>, big_component<3>>(registry, Size);
	add_entities<small_component<1>, big_component<4>>(registry, Size);
	add_entities<big_component<0>, medium_component<1>, small_component<1>>(registry, Size);
	add_entities<medium_component<0>, small_component<0>, small_component<1>>(registry, Size / 4);
	add_entities<big_component<0>, big_component<1>, big_component<2>, small_component<0>, medium_component<0>, small_component<1>>(
	    registry, Size / 4
	);
	add_entities<small_component<0>, small_component<2>>(registry, Size);
	add_entities<small_component<1>, small_component<2>>(registry, Size);
	add_entities<small_component<3>, small_component<2>>(registry, Size);
	add_entities<small_component<4>, small_component<2>>(registry, Size);
	add_entities<small_component<0>, small_component<1>, medium_component<0>, medium_component<1>, medium_component<2>>(registry, Size / 4);
	add_entities<small_component<5>, small_component<2>>(registry, Size);

	auto view = registry.view<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		view.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(registry);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void archetype_explosion_iteration(benchmark::State& state) {
	entt::registry registry;
	add_entities<small_component<0>, small_component<1>, medium_component<0>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<1>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<2>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<3>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<4>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<5>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<6>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<7>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<8>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<9>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<10>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<11>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<12>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<13>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<14>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<15>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<16>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<17>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<18>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<19>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<20>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<21>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<22>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<23>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<24>>(registry, Size / 26);
	add_entities<small_component<0>, small_component<1>, medium_component<25>>(registry, Size / 26);

	auto view = registry.view<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		view.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(registry);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

static void empty_entity_creation(benchmark::State& state) {
	entt::registry registry;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i)
			[[maybe_unused]] entt::entity e = registry.create();

		state.PauseTiming();
		registry.clear();
		benchmark::DoNotOptimize(registry);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void entity_creation(benchmark::State& state) {
	entt::registry registry;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i) {
			auto e = registry.create();
			registry.emplace<small_component<0>>(e);
			registry.emplace<small_component<1>>(e);
		}

		state.PauseTiming();
		registry.clear();
		registry.clear();
		benchmark::DoNotOptimize(registry);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void component_creation(benchmark::State& state) {
	entt::registry registry;
	std::vector<entt::entity> entities = add_entities<>(registry, 10000);
	for(auto _ : state) {
		for(entt::entity e : entities) registry.emplace<small_component<0>>(e);

		state.PauseTiming();
		for(entt::entity e : entities) registry.remove<small_component<0>>(e);
		benchmark::DoNotOptimize(registry);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void archetype_creation(benchmark::State& state) {
	for(auto _ : state) {
		entt::registry registry;
		registry.emplace<small_component<0>>(registry.create());
		registry.emplace<small_component<1>>(registry.create());
		registry.emplace<small_component<2>>(registry.create());
		registry.emplace<small_component<3>>(registry.create());
		registry.emplace<small_component<4>>(registry.create());
		registry.emplace<small_component<5>>(registry.create());
		registry.emplace<small_component<6>>(registry.create());
		registry.emplace<small_component<7>>(registry.create());
		registry.emplace<small_component<8>>(registry.create());
		registry.emplace<small_component<9>>(registry.create());
		benchmark::DoNotOptimize(registry);
	}
	state.SetItemsProcessed(10 * state.iterations());
}

// NOLINTBEGIN
#define CREATE_ITERATION_BENCHMARKS(name) \
	BENCHMARK(name<100>);                 \
	BENCHMARK(name<500>);                 \
	BENCHMARK(name<1000>);                \
	BENCHMARK(name<10000>);               \
	BENCHMARK(name<100000>)
// NOLINTEND

CREATE_ITERATION_BENCHMARKS(simple_scene_iteration);
CREATE_ITERATION_BENCHMARKS(complex_scene_iteration);
CREATE_ITERATION_BENCHMARKS(archetype_explosion_iteration);

BENCHMARK(empty_entity_creation);
BENCHMARK(entity_creation);
BENCHMARK(component_creation);
BENCHMARK(archetype_creation);

BENCHMARK_MAIN();
