#include <gtest/gtest.h>
#include <ecs/ecs.hpp>

TEST(type_info, same_type) {
    EXPECT_EQ(ecs::type_id<int>(), ecs::type_id<int>());
}

TEST(type_info, different_type) {
    EXPECT_NE(ecs::type_id<int>(), ecs::type_id<float>());
}

TEST(type_info, const_qualifier) {
    EXPECT_NE(ecs::type_id<int>(), ecs::type_id<const int>());
}
