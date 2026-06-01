#include <ecs/ecs.hpp>
#include <gtest/gtest.h>


TEST(type_info, same_type) {
	EXPECT_EQ(ecs::type_id<int>(), ecs::type_id<int>());
}

TEST(type_info, different_type) {
	EXPECT_NE(ecs::type_id<int>(), ecs::type_id<float>());
}

TEST(type_info, const_qualifier) {
	EXPECT_NE(ecs::type_id<int>(), ecs::type_id<const int>());
}
