#pragma once

#include <atomic>
#include <cstdint>
#include <type_traits>

#if defined(__clang__)
	#define RESTRICT __restrict__
#elif defined(__GNUC__) || defined(__GNUG__)
	#define RESTRICT __restrict__
#elif defined(_MSC_VER)
	#define RESTRICT __restrict
#else
	#define RESTRICT
#endif

namespace ecs {

    using type_index = uint32_t;
    using entity = uint32_t;
	using component_id = type_index;

	constexpr entity null_entity = entity(0);

    template<typename... T>
	struct is_unique;

	template<>
	struct is_unique<> : std::true_type {};

	template<typename T, typename... Rest>
	struct is_unique<T, Rest...> : std::bool_constant<!(std::is_same_v<T, Rest> || ...) && is_unique<Rest...>::value> {};

	template<typename... T>
	constexpr bool is_unique_v = is_unique<T...>::value;

	template<typename T>
	using component_reference = std::conditional_t<std::is_same_v<std::remove_const<T>, entity>, entity, T&>;

	namespace internal {
		[[nodiscard]] inline type_index next_type_index() noexcept {
			static std::atomic<type_index> idx = 0;
			return idx.fetch_add(1, std::memory_order_relaxed);
		}
	} // namespace internal

	template<typename T>
	[[nodiscard]] inline type_index type_id() noexcept {
		static type_index idx = internal::next_type_index();
		return idx;
	}

} // namespace ecs
