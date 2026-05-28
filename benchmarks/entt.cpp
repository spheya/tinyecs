#include <benchmark/benchmark.h>
#include <entt/entt.hpp>

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
		entt::registry world;
		for(int i = 0; i < 1000; ++i) {
			entt::entity entity = world.create();
			world.emplace<Position>(entity, Position{});
			world.emplace<Velocity>(entity, Velocity{});
			world.emplace<Renderer>(entity, Renderer{});
		}
		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_creation);

static void entity_destruction(benchmark::State& state) {
	for(auto _ : state) {
		state.PauseTiming();
		entt::registry world;
		std::vector<entt::entity> entities;
		entities.reserve(1000);
		for(int i = 0; i < 1000; ++i) {
			entities.push_back(world.create());
			world.emplace<Position>(entities.back(), Position{});
			world.emplace<Velocity>(entities.back(), Velocity{});
			world.emplace<Renderer>(entities.back(), Renderer{});
		}
		state.ResumeTiming();

		for(auto e : entities) world.destroy(e);
		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_destruction);

static void entity_iteration(benchmark::State& state) {
	entt::registry world;
	for(int i = 0; i < 1000; ++i) {
		entt::entity entity = world.create();
		world.emplace<Position>(entity, Position{});
		world.emplace<Velocity>(entity, float(i), float(i));
		world.emplace<Renderer>(entity, Renderer{});
	}

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [entity, pos, vel] : world.view<Position, Velocity>().each()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration);

static void entity_iteration_partial(benchmark::State& state) {
	entt::registry world;
	for(int i = 0; i < 1000; ++i) {
		entt::entity entity = world.create();
		world.emplace<Position>(entity, Position{});
		world.emplace<Velocity>(entity, float(i), float(i));
	}

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [entity, pos, vel] : world.view<Position, Velocity>().each()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_partial);

static void entity_iteration_const(benchmark::State& state) {
	entt::registry world;
	for(int i = 0; i < 1000; ++i) {
		entt::entity entity = world.create();
		world.emplace<Position>(entity, Position{});
		world.emplace<Velocity>(entity, float(i), float(i));
	}

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [entity, pos, vel] : world.view<const Position, const Velocity>().each()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_const);

static void entity_iteration_full_scene(benchmark::State& state) {
	entt::registry world;

	for(int i = 0; i < 500; ++i) {
		entt::entity a = world.create();
		world.emplace<Position>(a, Position{});
		world.emplace<Velocity>(a, float(i), float(i));

		entt::entity b = world.create();
		world.emplace<Position>(b, Position{});
		world.emplace<Velocity>(b, float(i), float(i));
		world.emplace<Renderer>(b, Renderer{});

		entt::entity c = world.create();
		world.emplace<Position>(c, Position{});

		entt::entity d = world.create();
		world.emplace<Position>(d, Position{});
		world.emplace<Renderer>(d, Renderer{});

		entt::entity e = world.create();
		world.emplace<Velocity>(e, Velocity{});
		world.emplace<Renderer>(e, Renderer{});

		entt::entity f = world.create();
		world.emplace<Velocity>(f, Velocity{});
	}

	for(auto _ : state) {
		float sum = 0.0f;
		for(auto&& [entity, pos, vel] : world.view<const Position, const Velocity>().each()) sum += pos.x + pos.y + vel.x + vel.y;
		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}
	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_full_scene);

static void entity_iteration_update(benchmark::State& state) {
	entt::registry world;
	for(int i = 0; i < 1000; ++i) {
		entt::entity entity = world.create();
		world.emplace<Position>(entity, Position{});
		world.emplace<Velocity>(entity, float(i), float(i));
	}
	for(auto _ : state) {
		for(auto&& [entity, pos, vel] : world.view<Position, const Velocity>().each()) {
			pos.x += vel.x;
			pos.y += vel.y;
		}
		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_update);

BENCHMARK_MAIN();
