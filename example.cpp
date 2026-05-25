#include <iostream>
#include <type_traits>

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
	ecs::entity entity1 = world.add_entity(Position{.x = 0.0f, .y = 0.0f, .z = 0.0f}, Physics{}, Renderer{});
	ecs::entity entity2 = world.add_entity(Position{.x = 1.0f, .y = 0.0f, .z = 0.0f}, Renderer{}, Physics{});
	ecs::entity entity3 = world.add_entity(Position{.x = 2.0f, .y = 0.0f, .z = 0.0f}, Renderer{});

	world.remove_entity(entity3);

	std::cout << "archetype count: " << world.archetypes.size() << '\n';

	std::cout << world.get_component<Position>(entity1).x << '\n';
	std::cout << world.get_component<Position>(entity2).x << '\n';
	//std::cout << world.get_component<Position>(entity3).x << '\n';
}
