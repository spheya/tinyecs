#pragma once

#include <tuple>
#include <type_traits>

namespace ecs {

	template<typename... T>
	class entity_view;

	template<typename T, typename... Rest>
	class entity_view<T, Rest...> {
	public:
	    entity_view(T* ptr, Rest* ...rest);

		template<size_t I>
		std::tuple_element_t<I, entity_view<T, Rest...>>& get() noexcept;

		template<size_t I>
		const std::tuple_element_t<I, entity_view<T, Rest...>>& get() const noexcept;

		template<typename Ty>
		Ty& get() noexcept;

		template<typename Ty>
		const Ty& get() const noexcept;

	private:
		T* m_ptr;
		entity_view<Rest...> m_rest;
	};

	template<typename T>
	class entity_view<T> {
	public:
	    entity_view(T* ptr);

		template<size_t I>
		T& get() noexcept;

		template<size_t I>
		const T& get() const noexcept;

		template<typename Ty>
		Ty& get() noexcept;

		template<typename Ty>
		const Ty& get() const noexcept;

	private:
		T* m_ptr;
	};

	template<typename T, typename... Rest>
    inline entity_view<T, Rest...>::entity_view(T* ptr, Rest* ... rest) : m_ptr(ptr), m_rest(rest...) {}

	template<typename T, typename... Rest>
	template<size_t I>
	inline std::tuple_element_t<I, entity_view<T, Rest...>>& entity_view<T, Rest...>::get() noexcept {
		if constexpr(I == 0) {
			return *m_ptr;
		} else {
			return m_rest.template get<I - 1>();
		}
	}

	template<typename T, typename... Rest>
	template<size_t I>
	inline const std::tuple_element_t<I, entity_view<T, Rest...>>& entity_view<T, Rest...>::get() const noexcept {
		if constexpr(I == 0) {
			return *m_ptr;
		} else {
			return m_rest.template get<I - 1>();
		}
	}

	template<typename T, typename... Rest>
	template<typename Ty>
	inline Ty& entity_view<T, Rest...>::get() noexcept {
		if constexpr(std::is_same_v<T, Ty>) {
			return *m_ptr;
		} else {
			return m_rest.template get<Ty>();
		}
	}

	template<typename T, typename... Rest>
	template<typename Ty>
	inline const Ty& entity_view<T, Rest...>::get() const noexcept {
		if constexpr(std::is_same_v<T, Ty>) {
			return *m_ptr;
		} else {
			return m_rest.template get<Ty>();
		}
	}

	template<typename T>
    inline entity_view<T>::entity_view(T* ptr) : m_ptr(ptr) {}

	template<typename T>
	template<size_t I>
	inline T& entity_view<T>::get() noexcept {
		static_assert(I == 0, "index out of range");
		return *m_ptr;
	}

	template<typename T>
	template<size_t I>
	inline const T& entity_view<T>::get() const noexcept {
    	static_assert(I == 0, "index out of range");
    	return *m_ptr;
	}

	template<typename T>
	template<typename Ty>
	inline Ty& entity_view<T>::get() noexcept {
	    static_assert(std::is_same_v<T, Ty>, "entity_view does not contain type");
		return *m_ptr;
	}

	template<typename T>
	template<typename Ty>
	inline const Ty& entity_view<T>::get() const noexcept {
        static_assert(std::is_same_v<T, Ty>, "entity_view does not contain type");
        return *m_ptr;
	}

} // namespace ecs

namespace std {

	template<typename... T>
	struct tuple_size<ecs::entity_view<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};

	template<typename... T, size_t I>
	struct tuple_element<I, ecs::entity_view<T...>> {
		using type = std::tuple_element_t<I, std::tuple<T...>>;
	};

} // namespace std
