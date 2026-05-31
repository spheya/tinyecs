#include <benchmark/benchmark.h>
#include <ecs/ecs.hpp>

#include "dummy_components.hpp"

#define CREATE_BENCHMARKS(name) \
	BENCHMARK(name<100>);       \
	BENCHMARK(name<500>);       \
	BENCHMARK(name<1000>);      \
	BENCHMARK(name<10000>);     \
	BENCHMARK(name<100000>)

template<size_t N, typename... T>
struct entity_layout {};

template<size_t N, typename... T>
static void spawn_entities(ecs::world& world, entity_layout<N, T...> /* layout */) {
	for(size_t i = 0; i < N; ++i) world.add_entity(init_component<T>()()...);
}

template<typename... T>
static ecs::world create_world() {
	ecs::world world;
	(spawn_entities(world, T{}), ...);
	return world;
}

template<size_t Size>
static void simple_scene(benchmark::State& state) {
	ecs::world world = create_world<entity_layout<Size, small_component<0>, small_component<1>>>();

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void complex_scene(benchmark::State& state) {
	ecs::world world = create_world<
	    entity_layout<Size / 4, small_component<0>, small_component<1>>,
	    entity_layout<Size, big_component<0>, big_component<1>>,
	    entity_layout<Size, small_component<0>, big_component<1>>,
	    entity_layout<Size, small_component<1>, big_component<1>>,
	    entity_layout<Size, small_component<1>, big_component<2>>,
	    entity_layout<Size, small_component<1>, big_component<3>>,
	    entity_layout<Size, small_component<1>, big_component<4>>,
	    entity_layout<Size, big_component<0>, medium_component<1>, small_component<1>>,
	    entity_layout<Size / 4, medium_component<0>, small_component<0>, small_component<1>>,
		entity_layout<Size / 4, big_component<0>, big_component<1>, big_component<2>, small_component<0>, medium_component<0>, small_component<1>>,
		entity_layout<Size, small_component<0>, small_component<2>>,
		entity_layout<Size, small_component<1>, small_component<2>>,
		entity_layout<Size, small_component<3>, small_component<2>>,
		entity_layout<Size, small_component<4>, small_component<2>>,
		entity_layout<Size / 4, small_component<0>, small_component<1>, medium_component<0>, medium_component<1>, medium_component<2>>,
		entity_layout<Size, small_component<5>, small_component<2>>
	>();

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void archetype_explosion(benchmark::State& state) {
	ecs::world world = create_world<
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<0>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<1>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<2>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<3>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<4>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<5>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<6>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<7>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<8>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<9>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<10>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<11>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<12>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<13>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<14>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<15>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<16>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<17>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<18>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<19>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<20>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<21>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<22>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<23>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<24>>,
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<25>>
	>();

	for(auto _ : state) {
		world.each([](small_component<0>& a, small_component<1> b) { a = a + b; });
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

CREATE_BENCHMARKS(simple_scene);
CREATE_BENCHMARKS(complex_scene);
CREATE_BENCHMARKS(archetype_explosion);

BENCHMARK_MAIN();
