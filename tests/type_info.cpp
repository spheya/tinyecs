#include <tinyecs/meta.hpp>
#include <gtest/gtest.h>

TEST(type_info, same_type) {
	EXPECT_EQ(tinyecs::type_id<int>(), tinyecs::type_id<int>());
}

TEST(type_info, different_type) {
	EXPECT_NE(tinyecs::type_id<int>(), tinyecs::type_id<float>());
}

TEST(type_info, const_qualifier) {
	EXPECT_NE(tinyecs::type_id<int>(), tinyecs::type_id<const int>());
}
