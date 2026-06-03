#include <gtest/gtest.h>
#include <tinyecs/world.hpp>

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
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Velocity>(entity));
	EXPECT_FALSE(world.has<Tag>(entity));

	auto&& [pos, vel] = world.get<Position, Velocity>(entity);
	EXPECT_EQ(pos.x, 1.0f);
	EXPECT_EQ(pos.y, 2.0f);
	EXPECT_EQ(vel.x, 3.0f);
	EXPECT_EQ(vel.y, 4.0f);
}

TEST(world, empty_entity) {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity();
	EXPECT_FALSE(world.has<Position>(entity));
}

TEST(world, modify_components) {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	world.get<Position>(entity).x = 67.0f;
	EXPECT_EQ(world.get<Position>(entity).x, 67.0f);
}

TEST(world, const_components) {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	EXPECT_EQ(world.get<const Position>(entity).x, 1.0f);
}

TEST(world, const_world) {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	const tinyecs::world& w = world;
	EXPECT_EQ(w.get<Position>(entity).x, 1.0f);
}

TEST(world, multiple_entities) {
	tinyecs::world world;
	tinyecs::entity entity1 = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	tinyecs::entity entity2 = world.create_entity(Position{ .x = 5.0f, .y = 6.0f }, Velocity{ .x = 7.0f, .y = 8.0f });

	EXPECT_TRUE(world.has<Position>(entity1));
	EXPECT_TRUE(world.has<Velocity>(entity1));
	EXPECT_FALSE(world.has<Tag>(entity1));
	EXPECT_TRUE(world.has<Position>(entity2));
	EXPECT_TRUE(world.has<Velocity>(entity2));
	EXPECT_FALSE(world.has<Tag>(entity2));

	auto&& [pos1, vel1] = world.get<Position, Velocity>(entity1);
	EXPECT_EQ(pos1.x, 1.0f);
	EXPECT_EQ(pos1.y, 2.0f);
	EXPECT_EQ(vel1.x, 3.0f);
	EXPECT_EQ(vel1.y, 4.0f);

	auto&& [pos2, vel2] = world.get<Position, Velocity>(entity2);
	EXPECT_EQ(pos2.x, 5.0f);
	EXPECT_EQ(pos2.y, 6.0f);
	EXPECT_EQ(vel2.x, 7.0f);
	EXPECT_EQ(vel2.y, 8.0f);
}

TEST(world, multiple_archetypes) {
	tinyecs::world world;
	tinyecs::entity entity1 = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	tinyecs::entity entity2 = world.create_entity(Position{ .x = 5.0f, .y = 6.0f });

	EXPECT_TRUE(world.has<Position>(entity1));
	EXPECT_TRUE(world.has<Velocity>(entity1));
	EXPECT_FALSE(world.has<Tag>(entity1));
	EXPECT_TRUE(world.has<Position>(entity2));
	EXPECT_FALSE(world.has<Velocity>(entity2));
	EXPECT_FALSE(world.has<Tag>(entity2));

	auto&& [pos1, vel1] = world.get<Position, Velocity>(entity1);
	EXPECT_EQ(pos1.x, 1.0f);
	EXPECT_EQ(pos1.y, 2.0f);
	EXPECT_EQ(vel1.x, 3.0f);
	EXPECT_EQ(vel1.y, 4.0f);

	auto& pos2 = world.get<Position>(entity2);
	EXPECT_EQ(pos2.x, 5.0f);
	EXPECT_EQ(pos2.y, 6.0f);
}

TEST(world, empty_components) {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(Tag{});
	EXPECT_TRUE(world.has<Tag>(entity));
}

TEST(world, entity_removal) {
	tinyecs::world world;
	tinyecs::entity remove = world.create_entity(Position{ .x = 5.0f, .y = 6.0f }, Velocity{ .x = 7.0f, .y = 8.0f });
	tinyecs::entity entity = world.create_entity(Position{ .x = 1.0f, .y = 2.0f }, Velocity{ .x = 3.0f, .y = 4.0f });
	world.remove_entity(remove);

	EXPECT_TRUE(world.has<Position>(entity));
	EXPECT_TRUE(world.has<Velocity>(entity));
	EXPECT_FALSE(world.has<Tag>(entity));

	auto&& [pos, vel] = world.get<Position, Velocity>(entity);
	EXPECT_EQ(pos.x, 1.0f);
	EXPECT_EQ(pos.y, 2.0f);
	EXPECT_EQ(vel.x, 3.0f);
	EXPECT_EQ(vel.y, 4.0f);
}
