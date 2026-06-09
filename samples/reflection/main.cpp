#include <format>
#include <iostream>

#include <tinyecs/tinyecs.hpp>

struct Position {
	float x, y;
};

struct Renderer {
	float r, g, b;
};

struct Extra {
	float f;
};

// The visit_component structs that will be called on `world::visit`, these just print out the data inside of the component for the example
namespace tinyecs {

	template<>
	struct visit_component<Position> {
		void operator()(void* /* user_data */, Position& position) { std::cout << std::format("Position: [x: {}, y: {}]\n", position.x, position.y); }
	};

	template<>
	struct visit_component<Renderer> {
		void operator()(void* /* user_data */, Renderer& renderer) {
			std::cout << std::format("Renderer: [r: {}, g: {}, b: {}]\n", renderer.r, renderer.g, renderer.b);
		}
	};

	// It will work without a generic one as well, just thought it would be nice to include it for the example
	template<typename T>
	struct visit_component<T> {
		void operator()(void* /* user_data */, T& /* t */) { std::cout << "Nothing implemented for this component\n"; }
	};

} // namespace tinyecs

int main() {
	ecs::world world;
	ecs::entity entity1 = world.create_entity(Position{ .x = 6.7f, .y = 42.0f }, Renderer{ .r = 1.0f, .g = 0.0f, .b = 1.0f });
	ecs::entity entity2 = world.create_entity(Position{ .x = 6.9f, .y = 0.0f });
	ecs::entity entity3 = world.create_entity(Extra{});

	// Prints out:
	// ```
	// Position: [x: 6.7, y: 42]
	// Renderer: [r: 1, g: 0, b: 1]
	// ```
	// note that the order in which visit_component structs are called is usually undefined
	world.visit(entity1, nullptr);
	std::cout << "\n";

	// Prints out:
	// ```
	// Position: [x: 6.9, y: 0]
	// ```
	world.visit(entity2, nullptr);
	std::cout << "\n";

	// Prints out:
	// ```
	// Nothing implemented for this component
	// ```
	world.visit(entity3, nullptr);

	return 0;
}
