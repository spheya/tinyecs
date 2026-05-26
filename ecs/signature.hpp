#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <type_traits>
#include <vector>

#include "ecs/type_info.hpp"
#include "types.hpp"

namespace ecs {

	struct signature {
		std::vector<component_id> components;

		template<typename... T>
		[[nodiscard]] bool contains() const noexcept;
		[[nodiscard]] bool contains(component_id component) const noexcept;

		template<typename T>
		[[nodiscard]] size_t index_of() const noexcept;
		[[nodiscard]] size_t index_of(component_id component) const noexcept;
		[[nodiscard]] size_t size() const noexcept;
	};

	template<typename... T>
	inline signature create_signature() {
		static_assert(is_unique_v<T...>, "Archetype signature must only contain unique types");
		static_assert((std::is_same_v<T, std::remove_cvref_t<T>> && ...), "Archetype signature can only contain non-reference, unqualified types");
		static_assert(!(std::is_same_v<T, entity> || ...), "Archetype signature cannot contain entity type");
		signature result{ { type_id<T>()... } };
		std::ranges::sort(result.components.begin(), result.components.end());
		return result;
	}

	template<typename... T>
	inline bool signature::contains() const noexcept {
		static_assert((std::is_same_v<T, std::remove_cvref_t<T>> && ...), "Archetype signature can only contain non-reference, unqualified types");
		static_assert(!(std::is_same_v<T, entity> || ...), "Archetype signature cannot contain entity type");
		return (contains(type_id<T>()) && ...);
	}

	inline bool signature::contains(component_id component) const noexcept {
		return std::ranges::find(components.begin(), components.end(), component)
		       != components.end(); // todo: binary search or sth, make use of the fact this list is sorted
	}

	template<typename T>
	inline size_t signature::index_of() const noexcept {
		static_assert(std::is_same_v<T, std::remove_cvref_t<T>>, "Archetype signature can only contain non-reference, unqualified types");
		static_assert(!std::is_same_v<T, entity>, "Archetype signature cannot contain entity type");
		return index_of(type_id<T>());
	}

	inline size_t signature::index_of(component_id component) const noexcept {
		auto it = std::ranges::find(components.begin(), components.end(), component);
		assert(it != components.end());
		return it - components.begin(); // todo: binary search or sth, make use of the fact this list is sorted
	}

	inline size_t signature::size() const noexcept {
		return components.size();
	}

	inline bool operator==(const signature& lhs, const signature& rhs) {
		return lhs.components == rhs.components;
	}

} // namespace ecs

template<>
struct std::hash<ecs::signature> {
	std::size_t operator()(const ecs::signature& signature) const noexcept {
		std::size_t hash = signature.components.size();
		for(auto i : signature.components) hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};
