#include <cstdlib>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <common/graphics.hpp>
#include <ecs/ecs.hpp>

struct position {
	float x, y;
};

struct velocity {
	float x, y;
};

static float random_float() {
	return float(rand()) / float(RAND_MAX);
}

static float random_float(float min, float max) {
	return random_float() * (max - min) + min;
}

int main() {
	graphics graphics("bouncing");
	ecs::world world;

	for(int i = 0; i < 500; ++i) {
		world.add_entity(
		    position{ .x = random_float(-540.0f, 540.0f), .y = random_float(-360.0f, 360.0f) },
		    velocity{ .x = random_float(-10.0f, 10.0f), .y = random_float(-10.0f, 10.0f) },
		    renderer{ .r = random_float(), .g = random_float(), .b = random_float(), .a = 1.0f }
		);
	}

	std::vector<instance> instances;
	while(!graphics.shouldClose()) {
		constexpr float dt = 1.0f / 60.0f;

		// movement
		for(auto&& [pos, vel] : world.view<position, velocity>()) {
			pos.x += vel.x * dt;
			pos.y += vel.y * dt;
		}

		// bounce
		for(auto&& [pos, vel] : world.view<position, velocity>()) {
			if(pos.x < -540.0f) {
				pos.x = -540.0f;
				vel.x = -vel.x;
			}

			if(pos.x > 540.0f) {
				pos.x = 540.0f;
				vel.x = -vel.x;
			}

			if(pos.y < -360.0f) {
				pos.y = -360.0f;
				vel.y = -vel.y;
			}

			if(pos.y > 360.0f) {
				pos.y = 360.0f;
				vel.y = -vel.y;
			}
		}

		// render
		instances.clear();
		for(auto&& [pos, render] : world.view<const position, const renderer>())
			instances.emplace_back(transform{ .pos_x = pos.x, .pos_y = pos.y, .size = 8.0f }, render);

		graphics.draw(instances);
	}
}
