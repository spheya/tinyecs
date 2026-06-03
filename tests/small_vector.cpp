#include <tinyecs/small_vector.hpp>
#include <gtest/gtest.h>

TEST(small_vector, push) {
	tinyecs::small_vector<int, 2> vec;
	vec.push_back(6);
	vec.push_back(7);
	EXPECT_EQ(vec[0], 6);
	EXPECT_EQ(vec[1], 7);
	EXPECT_EQ(vec.size(), 2);
}

TEST(small_vector, local_storage) {
	tinyecs::small_vector<int, 2> vec;
	vec.push_back(6);
	vec.push_back(7);
	void* local = vec.data();
	vec.push_back(2);
	EXPECT_EQ(vec[0], 6);
	EXPECT_EQ(vec[1], 7);
	EXPECT_EQ(vec[2], 2);
	EXPECT_NE(local, vec.data());
}

TEST(small_vector, capacity_growth) {
	tinyecs::small_vector<int, 2> vec;
	for(int i = 0; i < 100; ++i) vec.push_back(i);
	for(int i = 0; i < 100; ++i) EXPECT_EQ(vec[tinyecs::size_type(i)], i);
}

TEST(small_vector, resize) {
	tinyecs::small_vector<int, 4> vec;
	void* local = vec.data();
	vec.resize(4);
	EXPECT_EQ(local, vec.data());
	EXPECT_EQ(vec.size(), 4);
	vec.resize(2);
	EXPECT_EQ(local, vec.data());
	EXPECT_EQ(vec.size(), 2);
	vec.resize(20);
	EXPECT_NE(local, vec.data());
	EXPECT_EQ(vec.size(), 20);
}

TEST(small_vector, initializer_list) {
	tinyecs::small_vector<int, 4> vec = { 1, 2, 3, 4 };
	EXPECT_EQ(vec[0], 1);
	EXPECT_EQ(vec[1], 2);
	EXPECT_EQ(vec[2], 3);
	EXPECT_EQ(vec[3], 4);
	EXPECT_EQ(vec.size(), 4);
}

TEST(small_vector, copy_constructor) {
	tinyecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
	tinyecs::small_vector<int, 4> vec2 = vec1; // NOLINT

	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
	EXPECT_EQ(vec2[2], 3);
	EXPECT_EQ(vec2[3], 4);
	EXPECT_EQ(vec2.size(), 4);
}

TEST(small_vector, move_constructor) {
	tinyecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
	tinyecs::small_vector<int, 4> vec2 = std::move(vec1); // NOLINT

	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
	EXPECT_EQ(vec2[2], 3);
	EXPECT_EQ(vec2[3], 4);
	EXPECT_EQ(vec2.size(), 4);
}

TEST(small_vector, copy_assignment) {
	tinyecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
	tinyecs::small_vector<int, 4> vec2 = { 0, 1, 2, 3 };
	vec2 = vec1;

	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
	EXPECT_EQ(vec2[2], 3);
	EXPECT_EQ(vec2[3], 4);
	EXPECT_EQ(vec2.size(), 4);
}

TEST(small_vector, move_assignment) {
	tinyecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
	tinyecs::small_vector<int, 4> vec2 = { 0, 1, 2, 3 };
	vec2 = std::move(vec1);

	EXPECT_EQ(vec2[0], 1);
	EXPECT_EQ(vec2[1], 2);
	EXPECT_EQ(vec2[2], 3);
	EXPECT_EQ(vec2[3], 4);
	EXPECT_EQ(vec2.size(), 4);
}
