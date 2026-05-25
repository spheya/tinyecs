#pragma once

#include <algorithm>
#include <functional>
#include <vector>

#include "ecs/type_info.hpp"
#include "types.hpp"

namespace ecs {

	struct signature {
		std::vector<component_id> components;

		[[nodiscard]] size_t size() const noexcept { return components.size(); }

		[[nodiscard]] bool contains(component_id component) const {
			return std::ranges::find(components.begin(), components.end(), component)
			       != components.end(); // todo: binary search or sth, make use of the fact this list is sorted
		}

		[[nodiscard]] bool contains(const signature& other) const {
			return std::ranges::all_of(other.components, [this](component_id c){ return contains(c); });
		}

		[[nodiscard]] size_t getIndex(component_id component) const {
			return std::ranges::find(components.begin(), components.end(), component)
			       - components.begin(); // todo: binary search or sth, make use of the fact this list is sorted
		}
	};

	template<typename... T>
	inline signature create_signature() {
		signature result{ { type_id<T>()... } };
		std::ranges::sort(result.components.begin(), result.components.end());
		return result;
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
