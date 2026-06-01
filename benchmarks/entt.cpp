#include <benchmark/benchmark.h>
#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "dummy_components.hpp"

#define CREATE_BENCHMARKS(name) \
	BENCHMARK(name<100>);       \
	BENCHMARK(name<500>);       \
	BENCHMARK(name<1000>);      \
	BENCHMARK(name<10000>);
//	BENCHMARK(name<100000>)

template<size_t N, typename... T>
struct entity_layout {};

template<typename T>
static entt::entity spawn_entity(entt::registry& world) {
	entt::entity e = world.create();
	world.emplace<T>(e, init_component<T>()());
	return e;
}

template<typename T, typename S, typename... Rest>
static entt::entity spawn_entity(entt::registry& world) {
	entt::entity e = spawn_entity<S, Rest...>(world);
	world.emplace<T>(e, init_component<T>()());
	return e;
}

template<size_t N, typename... T>
static void spawn_entities(entt::registry& world, entity_layout<N, T...> /* layout */) {
	for(size_t i = 0; i < N; ++i) spawn_entity<T...>(world);
}

template<typename... T>
static entt::registry create_world() {
	entt::registry world;
	(spawn_entities(world, T{}), ...);
	return world;
}

template<size_t Size>
static void simple_scene(benchmark::State& state) {
	entt::registry world = create_world<entity_layout<Size, small_component<0>, small_component<1>>>();

	for(auto _ : state) {
		for(auto&& [entity, a, b] : world.view<small_component<0>, const small_component<1>>().each()) a = a + b;
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void complex_scene(benchmark::State& state) {
	entt::registry world = create_world<
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
	    entity_layout<Size, small_component<5>, small_component<2>>>();

	for(auto _ : state) {
		for(auto&& [entity, a, b] : world.view<small_component<0>, const small_component<1>>().each()) a = a + b;
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

template<size_t Size>
static void archetype_explosion(benchmark::State& state) {
	entt::registry world = create_world<
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
	    entity_layout<Size / 25, small_component<0>, small_component<1>, medium_component<25>>>();

	for(auto _ : state) {
		for(auto&& [entity, a, b] : world.view<small_component<0>, const small_component<1>>().each()) a = a + b;
		benchmark::DoNotOptimize(world);
	}

	state.SetItemsProcessed(int64_t(Size) * state.iterations());
}

CREATE_BENCHMARKS(simple_scene);
CREATE_BENCHMARKS(complex_scene);
CREATE_BENCHMARKS(archetype_explosion);

BENCHMARK_MAIN();
