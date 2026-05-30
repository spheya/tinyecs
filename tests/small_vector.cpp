#include <gtest/gtest.h>
#include <ecs/small_vector.hpp>

TEST(small_vector, push) {
    ecs::small_vector<int, 2> vec;
    vec.push_back(6);
    vec.push_back(7);
    EXPECT_EQ(vec[0], 6);
    EXPECT_EQ(vec[1], 7);
    EXPECT_EQ(vec.size(), 2);
}

TEST(small_vector, local_storage) {
    ecs::small_vector<int, 2> vec;
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
    ecs::small_vector<int, 2> vec;
    for (int i = 0; i < 100; ++i) vec.push_back(i);
    for (int i = 0; i < 100; ++i) EXPECT_EQ(vec[ecs::size_type(i)], i);
}

TEST(small_vector, resize) {
    ecs::small_vector<int, 4> vec;
    void* local = vec.data();
    vec.resize(4);
    EXPECT_EQ(local, vec.data());
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec.size(), 4);
    vec.resize(2);
    EXPECT_EQ(local, vec.data());
    EXPECT_EQ(vec[1], 0);
    EXPECT_EQ(vec.size(), 2);
    vec.resize(20);
    EXPECT_NE(local, vec.data());
    EXPECT_EQ(vec[18], 0);
    EXPECT_EQ(vec.size(), 20);
}

TEST(small_vector, initializer_list) {
    ecs::small_vector<int, 4> vec = { 1, 2, 3, 4 };
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[3], 4);
    EXPECT_EQ(vec.size(), 4);
}

TEST(small_vector, copy_constructor) {
    ecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
    ecs::small_vector<int, 4> vec2 = vec1; // NOLINT

    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
    EXPECT_EQ(vec2.size(), 4);
}

TEST(small_vector, move_constructor) {
    ecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
    ecs::small_vector<int, 4> vec2 = std::move(vec1); // NOLINT

    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
    EXPECT_EQ(vec2.size(), 4);
}

TEST(small_vector, copy_assignment) {
    ecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
    ecs::small_vector<int, 4> vec2 = { 0, 1, 2, 3 };
    vec2 = vec1;

    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
    EXPECT_EQ(vec2.size(), 4);
}


TEST(small_vector, move_assignment) {
    ecs::small_vector<int, 4> vec1 = { 1, 2, 3, 4 };
    ecs::small_vector<int, 4> vec2 = { 0, 1, 2, 3 };
    vec2 = std::move(vec1);

    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);
    EXPECT_EQ(vec2.size(), 4);
}
