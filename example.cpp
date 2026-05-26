#include <iostream>

#include <ecs/ecs.hpp>

struct Position {
	float x, y, z;
};

struct Physics {
	float vx, vy, vz;
};

struct Renderer {
	void* mesh;
};

int main() {
	ecs::world world;
	ecs::entity entity1 = world.add_entity(Position{ .x = 0.0f, .y = 0.0f, .z = 0.0f }, Physics{ .vx = 5.0f, .vy = 0.0f, .vz = 0.0f }, Renderer{});
	ecs::entity entity2 = world.add_entity(Position{ .x = 1.0f, .y = 0.0f, .z = 0.0f }, Renderer{}, Physics{ .vx = 4.0f, .vy = 0.0f, .vz = 0.0f });
	ecs::entity entity3 = world.add_entity(Position{ .x = 2.0f, .y = 0.0f, .z = 0.0f }, Renderer{});
	ecs::entity entity4 = world.add_entity(Position{ .x = 6.0f, .y = 0.0f, .z = 0.0f }, Physics{ .vx = 3.0f, .vy = 0.0f, .vz = 0.0f });

	auto view = world.view<const Position, Physics>();
	for(auto it = view.begin(); it != view.end(); ++it) {
		const Position& pos = it->get<0>();
		Physics& phys = it->get<1>();

		std::cout << pos.x << ' ';
		std::cout << phys.vx << '\n';
	}

	// std::cout << world.get_component<Position>(entity3).x << '\n';
}
