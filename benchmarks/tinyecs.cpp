#include <cstddef>
#include <cstdint>

#include <benchmark/benchmark.h>
#include <tinyecs/tinyecs.hpp>

#include "tests/common.hpp"
#include "tinyecs/world.hpp"

template<typename... Components>
static std::vector<tinyecs::entity> add_entities(tinyecs::world& world, size_t count) {
	std::vector<tinyecs::entity> results;
	for(size_t i = 0; i < count; ++i) {
		tinyecs::entity entity = world.create_entity(Components{}...);
		((world.get<Components>(entity) = component_generator<Components>{}(entity)), ...);
		results.push_back(entity);
	}
	return results;
}

template<size_t Size>
static void simple_scene_iteration(benchmark::State& state) {
	ecs::world world;
	add_entities<small_component<0>, small_component<1>>(world, Size);

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void complex_scene_iteration(benchmark::State& state) {
	ecs::world world;

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

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void archetype_explosion_iteration(benchmark::State& state) {
	ecs::world world;
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

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

static void empty_entity_creation(benchmark::State& state) {
	ecs::world world;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i) world.create_entity();
		state.PauseTiming();
		world.clear();
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void entity_creation(benchmark::State& state) {
	ecs::world world;
	for(auto _ : state) {
		for(int i = 0; i < 10000; ++i) world.create_entity(small_component<0>{}, small_component<1>{});
		state.PauseTiming();
		world.clear();
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void component_creation(benchmark::State& state) {
	ecs::world world;
	std::vector<ecs::entity> entities = add_entities<>(world, 10000);
	for(auto _ : state) {
		for(ecs::entity e : entities) world.add(e, small_component<0>{});

		state.PauseTiming();
		for(ecs::entity e : entities) world.remove<small_component<0>>(e);
		benchmark::DoNotOptimize(world);
		state.ResumeTiming();
	}
	state.SetItemsProcessed(10000 * state.iterations());
}

static void archetype_creation(benchmark::State& state) {
	for(auto _ : state) {
		ecs::world world;
		world.create_entity(small_component<0>{});
		world.create_entity(small_component<1>{});
		world.create_entity(small_component<2>{});
		world.create_entity(small_component<3>{});
		world.create_entity(small_component<4>{});
		world.create_entity(small_component<5>{});
		world.create_entity(small_component<6>{});
		world.create_entity(small_component<7>{});
		world.create_entity(small_component<8>{});
		world.create_entity(small_component<9>{});
		benchmark::DoNotOptimize(world);
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
