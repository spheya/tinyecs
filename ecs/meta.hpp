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

	using size_type = size_t;
	using type_index = uint32_t;
	using entity = uint32_t;
	using component_id = type_index;

	constexpr entity null_entity = entity(0);

	template<typename... T>
	struct function_args {};

	template<typename T>
	struct function_traits;

	template<typename Ret, typename Obj, typename... Args>
	struct function_traits<Ret (Obj::*)(Args...)> {
		using return_type = Ret;
		using object_type = Obj;
		using arguments = function_args<Args...>;
	};

	template<typename Ret, typename Obj, typename... Args>
	struct function_traits<Ret (Obj::*)(Args...) const> {
		using return_type = Ret;
		using object_type = Obj;
		using arguments = function_args<Args...>;
	};
	
	template<typename Ret, typename... Args>
	struct function_traits<Ret (*)(Args...)> {
		using return_type = Ret;
		using object_type = void;
		using arguments = function_args<Args...>;
	};

	template<typename T>
	struct function_traits : function_traits<decltype(&std::remove_cvref_t<T>::operator())> {};

	template<typename... T>
	struct is_unique;

	template<>
	struct is_unique<> : std::true_type {};

	template<typename T, typename... Rest>
	struct is_unique<T, Rest...> : std::bool_constant<!(std::is_same_v<T, Rest> || ...) && is_unique<Rest...>::value> {};

	template<typename... T>
	constexpr bool is_unique_v = is_unique<T...>::value;

	template<typename T>
	using reference = std::conditional_t<std::is_const_v<T> && std::is_trivially_copyable_v<T> && sizeof(T) <= 2 * sizeof(void*), T, T&>;

	template<typename T>
	using component_reference = std::conditional_t<std::is_same_v<std::remove_const<T>, entity>, entity, reference<T>>;

	inline static std::atomic<type_index> type_counter; // NOLINT

	template<typename T>
	[[nodiscard]] inline type_index type_id() noexcept {
		static const type_index idx = type_counter.fetch_add(1, std::memory_order_relaxed);
		return idx;
	}

} // namespace ecs
