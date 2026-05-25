#include <iostream>

#include <ecs/ecs.hpp>

int main() {
	ecs::world world;
	ecs::entity entity1 = world.add_entity<int, float, double>(5, 6.0f, 7.0);
	ecs::entity entity2 = world.add_entity<int, double, float>(5, 6.0, 7.0f);
	ecs::entity entity3 = world.add_entity<int, float>(5, 3.0f);

	std::cout << "archetype count: " << world.archetypes.size() << '\n';

	std::cout << world.get_component<float>(entity1) << '\n';
	std::cout << world.get_component<float>(entity2) << '\n';
	std::cout << world.get_component<float>(entity3) << '\n';
}
