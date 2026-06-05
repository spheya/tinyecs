#include <concepts>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>
#include <tinyecs/world.hpp>

#include "tinyecs/meta.hpp"

namespace {
	template<size_t N>
	struct component {
		int a, b, c;
		auto operator<=>(const component<N>&) const noexcept = default;
	};

	struct unique_component {
		std::unique_ptr<int> ptr;
		bool operator==(const unique_component& other) const { return *ptr == *other.ptr; }
	};

	struct tag {};

	template<typename T>
	struct component_generator;

	template<size_t N>
	struct component_generator<component<N>> {
		component<N> operator()(tinyecs::entity entity) {
			srand(entity + unsigned(N) * 2654418637u);
			return { .a = rand(), .b = rand(), .c = rand() };
		}
	};

	template<>
	struct component_generator<unique_component> {
		unique_component operator()(tinyecs::entity entity) {
			srand(entity);
			return { .ptr = std::make_unique<int>(rand()) };
		}
	};

	template<>
	struct component_generator<tag> {
		tag operator()(tinyecs::entity /* entity */) { return {}; }
	};
} // namespace

template<typename... Components>
static std::vector<tinyecs::entity> add_entities(tinyecs::world& world, size_t count) {
	std::vector<tinyecs::entity> results;
	for(size_t i = 0; i < count; ++i) {
		tinyecs::entity entity = world.create_entity(component_generator<Components>{}(tinyecs::null_entity)...);
		((world.get<Components>(entity) = component_generator<Components>{}(entity)), ...);
		results.push_back(entity);
	}
	return results;
}

template<typename T>
static void single_component_test() {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(component_generator<T>{}(0));
	EXPECT_TRUE(world.has<T>(entity));
	if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(entity), component_generator<T>{}(0));
}

template<typename T>
static void multi_component_test() {
	tinyecs::world world;
	tinyecs::entity entity = world.create_entity(component_generator<T>{}(0), component_generator<component<1>>{}(0));
	EXPECT_TRUE((world.has<T, component<1>>(entity)));
	if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(entity), component_generator<T>{}(0));
	EXPECT_EQ(world.get<component<1>>(entity), component_generator<component<1>>{}(0));
}

template<typename T>
static void multi_entity_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities<T, component<1>>(world, 1000);

	for(tinyecs::entity e : entities) {
		EXPECT_TRUE((world.has<T, component<1>>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
		EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
	}
}

template<typename T>
static void const_component_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities<T, component<1>>(world, 1000);

	for(tinyecs::entity e : entities) {
		EXPECT_TRUE((world.has<const T, const component<1>>(e)));
		EXPECT_EQ(world.get<const component<1>>(e), component_generator<component<1>>{}(e));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<const T>(e), component_generator<T>{}(e));
	}
}

template<typename T>
static void const_world_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities<T>(world, 1000);

	const tinyecs::world& w = world;

	for(tinyecs::entity e : entities) {
		EXPECT_TRUE((w.has<T>(e)));
		EXPECT_TRUE((w.has<const T>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(w.get<T>(e), component_generator<T>{}(e));
	}
}

template<typename T>
static void multi_archetype_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> e1 = add_entities<T>(world, 250);
	std::vector<tinyecs::entity> e2 = add_entities<T, component<1>>(world, 250);
	std::vector<tinyecs::entity> e3 = add_entities<T>(world, 250);
	std::vector<tinyecs::entity> e4 = add_entities<T, component<1>>(world, 250);

	for(tinyecs::entity e : e1) {
		EXPECT_TRUE((world.has<T>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
	}

	for(tinyecs::entity e : e2) {
		EXPECT_TRUE((world.has<T, component<1>>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
		EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
	}

	for(tinyecs::entity e : e3) {
		EXPECT_TRUE((world.has<T>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
	}

	for(tinyecs::entity e : e4) {
		EXPECT_TRUE((world.has<T, component<1>>(e)));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
		EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
	}
}

template<typename T>
static void add_components_after_creation_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities(world, 1000);
	for(tinyecs::entity e : entities) {
		world.add(e, component_generator<T>{}(e));
		if(e % 2 == 0) {
			world.add(e, component_generator<component<1>>{}(e));
		} else {
			world.add(e, component_generator<component<2>>{}(e));
		}
	}

	for(tinyecs::entity e : entities) {
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e));
		if(e % 2 == 0) {
			EXPECT_TRUE((world.has<T, component<1>>(e)));
			EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
		} else {
			EXPECT_TRUE((world.has<T, component<2>>(e)));
			EXPECT_EQ(world.get<component<2>>(e), component_generator<component<2>>{}(e));
		}
	}
}

template<typename T>
static void set_component_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities<T, component<1>>(world, 1000);
	for(tinyecs::entity e : entities) {
		if(e % 2 == 0) {
			world.set(e, component_generator<T>{}(0));
		} else {
			world.set(e, component_generator<T>{}(0), component_generator<component<2>>{}(e));
		}
	}

	for(tinyecs::entity e : entities) {
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<T>(e), component_generator<T>{}(0));
		EXPECT_TRUE(world.has<component<1>>(e));
		if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
		if(e % 2 != 0) {
			EXPECT_TRUE(world.has<component<2>>(e));
			if constexpr(std::equality_comparable<T>) EXPECT_EQ(world.get<component<2>>(e), component_generator<component<2>>{}(e));
		}
	}
}

template<typename T>
static void remove_component_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> entities = add_entities<T, component<1>>(world, 10);
	for(tinyecs::entity e : entities) {
		switch(e % 3) {
		case 0: world.remove<T>(e); break;
		case 1: world.remove<T, component<1>>(e); break;
		default: break;
		}
	}

	for(tinyecs::entity e : entities) {
		switch(e % 3) {
		case 0:
			EXPECT_FALSE(world.has<T>(e));
			EXPECT_TRUE(world.has<component<1>>(e));
			EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e));
			break;
		case 1: EXPECT_FALSE((world.has_any<T, component<1>>(e))); break;
		default:
			EXPECT_TRUE((world.has<T, component<1>>(e)));
			if constexpr(std::equality_comparable<T>) { 
				EXPECT_EQ(world.get<T>(e), component_generator<T>{}(e)); 
			}
			EXPECT_EQ(world.get<component<1>>(e), component_generator<component<1>>{}(e)); 
		}
	}
}

template<typename T>
static void simple_each_test() {
	tinyecs::world world;
	std::vector<tinyecs::entity> e1 = add_entities<T>(world, 500);
	add_entities<component<1>>(world, 500);
	std::vector<tinyecs::entity> e2 = add_entities<T, component<1>>(world, 500);
	add_entities<component<1>, component<2>>(world, 500);

	std::unordered_set<tinyecs::entity> entities(e1.begin(), e1.end());
	entities.insert(e2.begin(), e2.end());

	size_t counter = 0;
	world.each([&](tinyecs::entity entity, const T& /* component */) {
		++counter;
		ASSERT_TRUE(entities.contains(entity));
	});
	ASSERT_EQ(counter, 1000);
}

template<typename T>
static void entity_removal_test() {
	if constexpr(std::equality_comparable<T>) {
		tinyecs::world world;
		std::vector<tinyecs::entity> entities = add_entities<T>(world, 100);
		for(size_t i = 0; i < entities.size(); i += 2) world.remove_entity(entities[i]);
		for(size_t i = 1; i < entities.size(); i += 2) ASSERT_EQ(world.get<T>(entities[i]), component_generator<T>{}(entities[i]));
	}
}

#define SMOKE_TESTS(name, type)                         \
	TEST(world, name##_single_component) {              \
		single_component_test<type>();                  \
	}                                                   \
	TEST(world, name##_multi_component) {               \
		multi_component_test<type>();                   \
	}                                                   \
	TEST(world, name##_multi_entity) {                  \
		multi_entity_test<type>();                      \
	}                                                   \
	TEST(world, name##_const_component) {               \
		const_component_test<type>();                   \
	}                                                   \
	TEST(world, name##_const_world) {                   \
		const_world_test<type>();                       \
	}                                                   \
	TEST(world, name##_multi_archetype) {               \
		multi_archetype_test<type>();                   \
	}                                                   \
	TEST(world, name##_add_components_after_creation) { \
		add_components_after_creation_test<type>();     \
	}                                                   \
	TEST(world, name##_set_component_test) {            \
		set_component_test<type>();                     \
	}                                                   \
	TEST(world, name##_remove_component_test) {         \
		remove_component_test<type>();                  \
	}                                                   \
	TEST(world, name##_simple_each) {                   \
		simple_each_test<type>();                       \
	}                                                   \
	TEST(world, name##_entity_removal) {                \
		entity_removal_test<type>();                    \
	}

SMOKE_TESTS(basic, component<0>);
SMOKE_TESTS(non_movable, unique_component);
SMOKE_TESTS(tag, tag);
