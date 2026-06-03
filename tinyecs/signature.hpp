#pragma once

#include <algorithm>
#include <cstring>
#include <functional>
#include <type_traits>

#include "meta.hpp"
#include "small_vector.hpp"

namespace tinyecs {

	struct signature {
		small_vector<component_id, 4> components;
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
	inline signature extend_signature(signature s) {
		static_assert(is_unique_v<T...>, "Archetype signature must only contain unique types");
		static_assert((std::is_same_v<T, std::remove_cvref_t<T>> && ...), "Archetype signature can only contain non-reference, unqualified types");
		static_assert(!(std::is_same_v<T, entity> || ...), "Archetype signature cannot contain entity type");
		signature result{ std::move(s.components) };
		result.components.reserve(s.components.size() + sizeof...(T));
		(result.components.push_back(type_id<T>()), ...);
		std::ranges::sort(result.components.begin(), result.components.end());
		TINYECS_ASSUME(std::ranges::adjacent_find(result.components.begin(), result.components.end()) == result.components.end());
		return result;
	}

	inline bool operator==(const signature& lhs, const signature& rhs) {
		if(lhs.components.size() != rhs.components.size()) return false;
		return memcmp(lhs.components.data(), rhs.components.data(), lhs.components.size() * sizeof(component_id)) == 0;
	}

	inline bool operator!=(const signature& lhs, const signature& rhs) {
		if(lhs.components.size() != rhs.components.size()) return false;
		return memcmp(lhs.components.data(), rhs.components.data(), lhs.components.size() * sizeof(component_id)) == 0;
	}

} // namespace tinyecs

template<>
struct std::hash<tinyecs::signature> {
	std::size_t operator()(const tinyecs::signature& signature) const noexcept {
		std::size_t hash = signature.components.size();
		for(auto i : signature.components) hash ^= i + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		return hash;
	}
};
