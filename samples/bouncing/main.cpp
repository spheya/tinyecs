#include <cstdlib>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <common/graphics.hpp>
#include <ecs/ecs.hpp>

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
		    transform{ .pos_x = random_float(-540.0f, 540.0f), .pos_y = random_float(-360.0f, 360.0f), .size = 12.0f },
		    velocity{ .x = random_float(-10.0f, 10.0f), .y = random_float(-10.0f, 10.0f) },
		    renderer{ .r = random_float(), .g = random_float(), .b = random_float(), .a = 1.0f }
		);
	}

	std::vector<instance> instances;
	while(!graphics.shouldClose()) {
		constexpr float dt = 1.0f / 60.0f;

		// movement
		for(auto&& [trans, vel] : world.view<transform, velocity>()) {
			trans.pos_x += vel.x * dt;
			trans.pos_y += vel.y * dt;
		}

		// bounce
		for(auto&& [trans, vel] : world.view<transform, velocity>()) {
			if(trans.pos_x < -540.0f) {
				trans.pos_x = -540.0f;
				vel.x = -vel.x;
			}

			if(trans.pos_x > 540.0f) {
				trans.pos_x = 540.0f;
				vel.x = -vel.x;
			}

			if(trans.pos_y < -360.0f) {
				trans.pos_y = -360.0f;
				vel.y = -vel.y;
			}

			if(trans.pos_y > 360.0f) {
				trans.pos_y = 360.0f;
				vel.y = -vel.y;
			}
		}

		// render
		instances.clear();
		for(auto&& [trans, render] : world.view<const transform, const renderer>())
			instances.emplace_back(transform{ trans }, render);

		graphics.draw(instances);
	}
}
