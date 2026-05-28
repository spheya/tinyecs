#include <benchmark/benchmark.h>
#include <ecs/ecs.hpp>

struct Position {
	float x, y;
};

struct Velocity {
	float x, y;
};

struct Renderer {
	float r, g, b, a;
	float scale;
};

static void entity_creation(benchmark::State& state) {
	for(auto _ : state) {
		ecs::world world;
		for(int i = 0; i < 1000; ++i) world.add_entity(Position{}, Velocity{}, Renderer{});
		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_creation);

static void entity_destruction(benchmark::State& state) {
	for(auto _ : state) {
		state.PauseTiming();
		ecs::world world;
		std::vector<ecs::entity> entities;
		entities.reserve(1000);
		for(int i = 0; i < 1000; ++i) entities.push_back(world.add_entity(Position{}, Velocity{}, Renderer{}));
		state.ResumeTiming();

		for(auto e : entities) world.remove_entity(e);
		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_destruction);

static void entity_iteration(benchmark::State& state) {
	ecs::world world;
	for(int i = 0; i < 1000; ++i) world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) });

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [pos, vel] : world.view<Position, Velocity>()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration);

static void entity_iteration_partial(benchmark::State& state) {
	ecs::world world;
	for(int i = 0; i < 1000; ++i) world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) }, Renderer{});

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [pos, vel] : world.view<Position, Velocity>()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_partial);

static void entity_iteration_const(benchmark::State& state) {
	ecs::world world;
	for(int i = 0; i < 1000; ++i) world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) });

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [pos, vel] : world.view<const Position, const Velocity>()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_const);

static void entity_iteration_full_scene(benchmark::State& state) {
	ecs::world world;
	for(int i = 0; i < 500; ++i) {
		world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) });
		world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) }, Renderer{});
		world.add_entity(Position{});
		world.add_entity(Position{}, Renderer{});
		world.add_entity(Velocity{}, Renderer{});
		world.add_entity(Renderer{});
	}

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [pos, vel] : world.view<const Position, const Velocity>()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_full_scene);

static void entity_iteration_update(benchmark::State& state) {
	ecs::world world;
	for(int i = 0; i < 1000; ++i) world.add_entity(Position{}, Velocity{ .x = float(i), .y = float(i) });

	for(auto _ : state) {
		for(auto&& [pos, vel] : world.view<Position, const Velocity>()) {
			pos.x += vel.x;
			pos.y += vel.y;
		}
	}
	benchmark::DoNotOptimize(world);
	benchmark::ClobberMemory();
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_update);

BENCHMARK_MAIN();
