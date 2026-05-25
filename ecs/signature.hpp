#pragma once

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>

#include "ecs/type_info.hpp"
#include "types.hpp"

namespace ecs {

	template<typename... T>
	struct valid_signature;

	template<>
	struct valid_signature<> : std::true_type {};

	template<typename T, typename... Rest>
	struct valid_signature<T, Rest...> :
	    std::bool_constant<!(std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<Rest>> || ...) && valid_signature<Rest...>::value> {};

	template<typename... T>
	constexpr bool valid_signature_v = valid_signature<T...>::value;

	struct signature {
		std::vector<component_id> components;

		template<typename... T>
		[[nodiscard]] bool contains() const;
		[[nodiscard]] bool contains(component_id component) const;
		[[nodiscard]] size_t getIndex(component_id component) const;
		[[nodiscard]] size_t size() const noexcept;
	};

	template<typename... T>
	    requires valid_signature_v<T...>
	inline signature create_signature() {
		signature result{ { type_id<T>()... } };
		std::ranges::sort(result.components.begin(), result.components.end());
		return result;
	}

	template<typename... T>
	inline bool signature::contains() const {
		return (contains(type_id<T>()) && ...);
	}

	inline bool signature::contains(component_id component) const {
		return std::ranges::find(components.begin(), components.end(), component)
		       != components.end(); // todo: binary search or sth, make use of the fact this list is sorted
	}

	inline size_t signature::getIndex(component_id component) const {
		return std::ranges::find(components.begin(), components.end(), component)
		       - components.begin(); // todo: binary search or sth, make use of the fact this list is sorted
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
