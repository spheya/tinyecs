#include <ecs/ecs.hpp>
#include <gtest/gtest.h>

namespace {
	struct Position {
		float x, y;
	};

	struct Velocity {
		float x, y;
	};

	struct Tag {};
} // namespace

TEST(world, single_entity) {
	ecs::world world;
	ecs::entity entity = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });

	EXPECT_TRUE(world.has_component<Position>(entity));
	EXPECT_TRUE(world.has_component<Velocity>(entity));
	EXPECT_FALSE(world.has_component<Tag>(entity));

	auto&& [pos, vel] = world.get_components<Position, Velocity>(entity);
	EXPECT_EQ(pos.x, 1.0f);
	EXPECT_EQ(pos.y, 2.0f);
	EXPECT_EQ(vel.x, 3.0f);
	EXPECT_EQ(vel.y, 4.0f);
}

TEST(world, empty_entity) {
	ecs::world world;
	ecs::entity entity = world.add_entity();
	EXPECT_FALSE(world.has_component<Position>(entity));
}

TEST(world, modify_components) {
	ecs::world world;
	ecs::entity entity = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	world.get_component<Position>(entity).x = 67.0f;
	EXPECT_EQ(world.get_component<Position>(entity).x, 67.0f);
}

TEST(world, const_components) {
	ecs::world world;
	ecs::entity entity = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	EXPECT_EQ(world.get_component<const Position>(entity).x, 1.0f);
}

TEST(world, const_world) {
	ecs::world world;
	ecs::entity entity = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	const ecs::world& w = world;
	EXPECT_EQ(w.get_component<Position>(entity).x, 1.0f);
}

TEST(world, multiple_entities) {
	ecs::world world;
	ecs::entity entity1 = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	ecs::entity entity2 = world.add_entity(Position{ .x = 5.0f, .y = 6.0f }, Velocity{ .x = 7.0f, .y = 8.0f });

	EXPECT_TRUE(world.has_component<Position>(entity1));
	EXPECT_TRUE(world.has_component<Velocity>(entity1));
	EXPECT_FALSE(world.has_component<Tag>(entity1));
	EXPECT_TRUE(world.has_component<Position>(entity2));
	EXPECT_TRUE(world.has_component<Velocity>(entity2));
	EXPECT_FALSE(world.has_component<Tag>(entity2));

	auto&& [pos1, vel1] = world.get_components<Position, Velocity>(entity1);
	EXPECT_EQ(pos1.x, 1.0f);
	EXPECT_EQ(pos1.y, 2.0f);
	EXPECT_EQ(vel1.x, 3.0f);
	EXPECT_EQ(vel1.y, 4.0f);

	auto&& [pos2, vel2] = world.get_components<Position, Velocity>(entity2);
	EXPECT_EQ(pos2.x, 5.0f);
	EXPECT_EQ(pos2.y, 6.0f);
	EXPECT_EQ(vel2.x, 7.0f);
	EXPECT_EQ(vel2.y, 8.0f);
}

TEST(world, multiple_archetypes) {
	ecs::world world;
	ecs::entity entity1 = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	ecs::entity entity2 = world.add_entity(Position{ .x = 5.0f, .y = 6.0f });

	EXPECT_TRUE(world.has_component<Position>(entity1));
	EXPECT_TRUE(world.has_component<Velocity>(entity1));
	EXPECT_FALSE(world.has_component<Tag>(entity1));
	EXPECT_TRUE(world.has_component<Position>(entity2));
	EXPECT_FALSE(world.has_component<Velocity>(entity2));
	EXPECT_FALSE(world.has_component<Tag>(entity2));

	auto&& [pos1, vel1] = world.get_components<Position, Velocity>(entity1);
	EXPECT_EQ(pos1.x, 1.0f);
	EXPECT_EQ(pos1.y, 2.0f);
	EXPECT_EQ(vel1.x, 3.0f);
	EXPECT_EQ(vel1.y, 4.0f);

	auto& pos2 = world.get_component<Position>(entity2);
	EXPECT_EQ(pos2.x, 5.0f);
	EXPECT_EQ(pos2.y, 6.0f);
}

TEST(world, empty_components) {
	ecs::world world;
	ecs::entity entity = world.add_entity(Tag{});
	EXPECT_TRUE(world.has_component<Tag>(entity));
}

TEST(world, entity_removal) {
	ecs::world world;
	ecs::entity remove = world.add_entity(Position{ .x = 5.0f, .y = 6.0f }, Velocity{ .x = 7.0f, .y = 8.0f });
	ecs::entity entity = world.add_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	world.remove_entity(remove);

	EXPECT_TRUE(world.has_component<Position>(entity));
	EXPECT_TRUE(world.has_component<Velocity>(entity));
	EXPECT_FALSE(world.has_component<Tag>(entity));

	auto&& [pos, vel] = world.get_components<Position, Velocity>(entity);
	EXPECT_EQ(pos.x, 1.0f);
	EXPECT_EQ(pos.y, 2.0f);
	EXPECT_EQ(vel.x, 3.0f);
	EXPECT_EQ(vel.y, 4.0f);
}
