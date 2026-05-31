#include <vector>

#include <benchmark/benchmark.h>
#include <flecs.h>

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
		flecs::world world;
		for(int i = 0; i < 1000; ++i) world.entity().set<Position>({}).set<Velocity>({}).set<Renderer>({});

		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_creation);

static void entity_destruction(benchmark::State& state) {
	for(auto _ : state) {
		state.PauseTiming();
		flecs::world world;
		std::vector<flecs::entity> entities;
		entities.reserve(1000);
		for(int i = 0; i < 1000; ++i) entities.push_back(world.entity().set<Position>({}).set<Velocity>({}).set<Renderer>({}));
		state.ResumeTiming();

		for(auto e : entities) e.destruct();

		benchmark::DoNotOptimize(world);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_destruction);

static void entity_iteration(benchmark::State& state) {
	flecs::world world;
	for(int i = 0; i < 1000; ++i) world.entity().set<Position>({}).set<Velocity>({ .x = float(i), .y = float(i) });

	auto q = world.query<Position, Velocity>();

	for(auto _ : state) {
		float sum = 0.0f;

		q.each([&](Position pos, Velocity vel) { sum += pos.x + pos.y + vel.x + vel.y; });

		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration);

static void entity_iteration_partial(benchmark::State& state) {
	flecs::world world;
	for(int i = 0; i < 1000; ++i) world.entity().set<Position>({}).set<Velocity>({ .x = float(i), .y = float(i) }).set<Renderer>({});

	auto q = world.query<Position, Velocity>();

	for(auto _ : state) {
		float sum = 0.0f;

		q.each([&](Position pos, Velocity vel) { sum += pos.x + pos.y + vel.x + vel.y; });

		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_partial);

static void entity_iteration_const(benchmark::State& state) {
	flecs::world world;
	for(int i = 0; i < 1000; ++i) world.entity().set<Position>({}).set<Velocity>({ .x = float(i), .y = float(i) });

	auto q = world.query<const Position, const Velocity>();

	for(auto _ : state) {
		float sum = 0.0f;

		q.each([&](Position pos, Velocity vel) { sum += pos.x + pos.y + vel.x + vel.y; });

		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_const);

static void entity_iteration_full_scene(benchmark::State& state) {
	flecs::world world;
	for(int i = 0; i < 500; ++i) {
		world.entity().set<Position>({});
		world.entity().set<Position>({}).set<Velocity>({ .x = float(i), .y = float(i) });
		world.entity().set<Position>({}).set<Renderer>({});
		world.entity().set<Velocity>({}).set<Renderer>({});
		world.entity().set<Renderer>({});
	}

	auto q = world.query<const Position, const Velocity>();

	for(auto _ : state) {
		float sum = 0.0f;

		q.each([&](Position pos, Velocity vel) { sum += pos.x + pos.y + vel.x + vel.y; });

		benchmark::DoNotOptimize(sum);
		benchmark::ClobberMemory();
	}

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_full_scene);

static void entity_iteration_update(benchmark::State& state) {
	flecs::world world;
	for(int i = 0; i < 1000; ++i) world.entity().set<Position>({}).set<Velocity>({ .x = float(i), .y = float(i) });

	auto q = world.query<Position, const Velocity>();

	for(auto _ : state) {
		q.each([](Position& pos, Velocity vel) {
			pos.x += vel.x;
			pos.y += vel.y;
		});
	}

	benchmark::DoNotOptimize(world);
	benchmark::ClobberMemory();

	state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(entity_iteration_update);

BENCHMARK_MAIN();
