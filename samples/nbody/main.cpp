#include <cmath>
#include <cstdlib>

#include "ecs/meta.hpp"

#define GLAD_GL_IMPLEMENTATION
#include <common/graphics.hpp>
#include <ecs/ecs.hpp>

struct body {
	double pos_x, pos_y;
	double mass;
};

struct velocity {
	double x, y;
	double startx, starty;
};

static float random_float() {
	return float(rand()) / float(RAND_MAX);
}

static float random_float(float min, float max) {
	return random_float() * (max - min) + min;
}

int main() {
	graphics graphics("nbody");
	ecs::world world;

	for(int i = 0; i < 1000; ++i) {
		world.add_entity(
		    body{ .pos_x = random_float(-2000.0f, 2000.0f), .pos_y = random_float(-2000.0f, 2000.0f), .mass = pow(random_float(), 10.0f) * 10000.0 + 10.0f },
		    velocity{ .x = random_float(-10000.0f, 10000.0f), .y = random_float(-10000.0f, 10000.0f) },
		    renderer{ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f }
		);
	}
	ecs::entity sun = world.add_entity(
	    body{ .pos_x = 0.0, .pos_y = 0.0, .mass = 80000.0 }, renderer{ .r = 1.0f, .g = 1.0f, .b = 1.0f, .a = 1.0f }, velocity{ .x = 0.0f, .y = 0.0f }
	);

	std::vector<instance> instances;
	std::vector<ecs::entity> removal;
	while(!graphics.shouldClose()) {
		constexpr double dt = 0.01 / 60.0;
		constexpr double grav_constant = 50000.0;

		// Update velocities
		for(auto&& [body1, vel1] : world.view<const body, velocity>()) {
			vel1.startx = vel1.x;
			for(auto&& [body2] : world.view<const body>()) {
				double dx = body2.pos_x - body1.pos_x;
				double dy = body2.pos_y - body1.pos_y;
				double distSq = dx * dx + dy * dy;
				if(distSq < 0.0001) continue;
				double dist = sqrt(distSq);
				double grav = grav_constant * body2.mass / distSq;
				vel1.x += dt * dx * grav / dist;
				vel1.y += dt * dy * grav / dist;
			}
		}

		// Move objects
		for(auto&& [body, vel] : world.view<body, const velocity>()) {
			body.pos_x += (vel.x + vel.startx) * dt * 0.5;
			body.pos_y += (vel.y + vel.starty) * dt * 0.5;
		}

		// Handle collisions
		removal.clear();
		auto view = world.view<ecs::entity, body, velocity>();
		for(auto it = view.begin(); it != view.end(); ++it) {
			auto&& [entity1, body1, velocity1] = *it;
			for(auto it2 = it; it2 != view.end(); ++it2) {
				auto&& [entity2, body2, velocity2] = *it2;
				if(entity1 == entity2) continue;
				double dx = body2.pos_x - body1.pos_x;
				double dy = body2.pos_y - body1.pos_y;
				double distSq = dx * dx + dy * dy;
				double maxDist = 0.5 * (pow(body1.mass, 0.333f) + pow(body2.mass, 0.333f));
				if(distSq < maxDist * maxDist) {
					body2.mass += body1.mass;
					body2.pos_x = std::lerp(body2.pos_x, body1.pos_x, body1.mass / body2.mass);
					body2.pos_y = std::lerp(body2.pos_y, body1.pos_y, body1.mass / body2.mass);
					velocity2.x = std::lerp(velocity2.x, velocity1.x, body1.mass / body2.mass);
					velocity2.y = std::lerp(velocity2.y, velocity1.y, body1.mass / body2.mass);
					removal.push_back(entity1);
					break;
				}
			}
		}
		for(ecs::entity e : removal) world.remove_entity(e);

		double highest_mass = 0.0;
		double highest_mass_x = 0.0;
		double highest_mass_y = 0.0;
		for(auto&& [body] : world.view<const body>()) {
			if(body.mass > highest_mass) {
				highest_mass = body.mass;
				highest_mass_x = body.pos_x;
				highest_mass_y = body.pos_y;
			}
		};

		instances.clear();
		for(auto&& [body, renderer] : world.view<const body, const renderer>())
			instances.emplace_back(
			    transform{ .pos_x = float(body.pos_x - highest_mass_x) * 0.07f, .pos_y = float(body.pos_y - highest_mass_y) * 0.07f, .size = float(pow(body.mass, 0.333f) * 0.5) },
			    renderer
			);
		graphics.draw(instances);
	}
}
