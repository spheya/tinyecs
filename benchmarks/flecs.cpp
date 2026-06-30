#include <cstddef>
#include <cstdint>

#include <benchmark/benchmark.h>
#include <flecs.h>

#include "tests/common.hpp"

template<typename... Components>
static std::vector<flecs::entity> add_entities(flecs::world& world, size_t count) {
	std::vector<flecs::entity> results;
	results.reserve(count);

	for(size_t i = 0; i < count; ++i) {
		flecs::entity entity = world.entity();
		((entity.set<Components>(component_generator<Components>{}(uint64_t(i)))), ...);
		results.push_back(entity);
	}
	return results;
}

template<size_t Size>
static void simple_scene_iteration(benchmark::State& state) {
	flecs::world world;
	add_entities<small_component<0>, small_component<1>>(world, Size);

	auto q = world.query<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		q.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void complex_scene_iteration(benchmark::State& state) {
	flecs::world world;

	add_entities<small_component<0>, small_component<1>>(world, Size / 4);
	add_entities<big_component<0>, big_component<1>>(world, Size);
	add_entities<small_component<0>, big_component<1>>(world, Size);
	add_entities<small_component<1>, big_component<1>>(world, Size);
	add_entities<small_component<1>, big_component<2>>(world, Size);
	add_entities<small_component<1>, big_component<3>>(world, Size);
	add_entities<small_component<1>, big_component<4>>(world, Size);
	add_entities<big_component<0>, medium_component<1>, small_component<1>>(world, Size);
	add_entities<medium_component<0>, small_component<0>, small_component<1>>(world, Size / 4);
	add_entities<big_component<0>, big_component<1>, big_component<2>, small_component<0>, medium_component<0>, small_component<1>>(world, Size / 4);
	add_entities<small_component<0>, small_component<2>>(world, Size);
	add_entities<small_component<1>, small_component<2>>(world, Size);
	add_entities<small_component<3>, small_component<2>>(world, Size);
	add_entities<small_component<4>, small_component<2>>(world, Size);
	add_entities<small_component<0>, small_component<1>, medium_component<0>, medium_component<1>, medium_component<2>>(world, Size / 4);
	add_entities<small_component<5>, small_component<2>>(world, Size);

	auto q = world.query<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		q.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void archetype_explosion_iteration(benchmark::State& state) {
	flecs::world world;
	add_entities<small_component<0>, small_component<1>, medium_component<0>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<1>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<2>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<3>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<4>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<5>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<6>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<7>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<8>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<9>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<10>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<11>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<12>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<13>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<14>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<15>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<16>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<17>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<18>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<19>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<20>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<21>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<22>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<23>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<24>>(world, Size / 25);
	add_entities<small_component<0>, small_component<1>, medium_component<25>>(world, Size / 25);

	auto q = world.query<small_component<0>, small_component<1>>();

	for(auto _ : state) {
		q.each([](small_component<0>& a, small_component<1>& b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

static void empty_entity_creation(benchmark::State& state) {
	flecs::world world;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i) world.entity();
		state.PauseTiming();
		world.delete_with(0);
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void entity_creation(benchmark::State& state) {
	flecs::world world;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i) world.entity().set<small_component<0>>({}).set<small_component<1>>({});
		state.PauseTiming();
		world.delete_with<small_component<0>>();
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void component_creation(benchmark::State& state) {
	flecs::world world;
	std::vector<flecs::entity> entities = add_entities<>(world, 10000);
	for(auto _ : state) {
		for(flecs::entity e : entities) e.set<small_component<0>>({});

		state.PauseTiming();
		for(flecs::entity e : entities) e.remove<small_component<0>>();
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void archetype_creation(benchmark::State& state) {
	for(auto _ : state) {
		flecs::world world;
		world.entity().set<small_component<0>>({});
		world.entity().set<small_component<1>>({});
		world.entity().set<small_component<2>>({});
		world.entity().set<small_component<3>>({});
		world.entity().set<small_component<4>>({});
		world.entity().set<small_component<5>>({});
		world.entity().set<small_component<6>>({});
		world.entity().set<small_component<7>>({});
		world.entity().set<small_component<8>>({});
		world.entity().set<small_component<9>>({});
		benchmark::DoNotOptimize(world);
	}
	state.SetItemsProcessed(10 * state.iterations());
}

static void individual_component_access(benchmark::State& state) {
	flecs::world world;
	flecs::entity e = world.entity().set(small_component<0>{});
	for(auto _ : state) {
		world.set(e, small_component<0>{});
		benchmark::DoNotOptimize(world);
	}
	state.SetItemsProcessed(state.iterations());
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
BENCHMARK(individual_component_access);

BENCHMARK_MAIN();
