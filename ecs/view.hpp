#pragma once

#include <tuple>
#include <type_traits>

#include "archetype.hpp"
#include "small_vector.hpp"

namespace ecs {

	template<typename... T>
	class entity_view;

	template<typename T, typename... Rest>
	class entity_view<T, Rest...> {
		template<typename... Ty>
		friend class view_iterator;

		static_assert(!(std::is_same_v<std::remove_const_t<T>, std::remove_const_t<Rest>> || ...), "View must only contain unique types");
		static_assert(std::is_same_v<std::remove_const_t<T>, std::remove_cvref_t<T>>, "View must only contain non-reference and non-volatile types");

	public:
		entity_view() noexcept = default;
		entity_view(T* ptr, Rest*... rest) noexcept;

		template<size_t I>
		component_reference<std::tuple_element_t<I, entity_view<T, Rest...>>> get() const noexcept;

		template<typename Ty>
		component_reference<Ty> get() const noexcept;

		entity_view<T, Rest...>& operator++() noexcept;
		entity_view<T, Rest...>& operator--() noexcept;

		bool operator==(const entity_view<T, Rest...>& other) const noexcept;
		bool operator!=(const entity_view<T, Rest...>& other) const noexcept;

	private:
		T* m_ptr = nullptr;
		entity_view<Rest...> m_rest;
	};

	template<typename T>
	class entity_view<T> {
		template<typename... Ty>
		friend class view_iterator;

		static_assert(std::is_same_v<std::remove_const_t<T>, std::remove_cvref_t<T>>, "View must only contain non-reference and non-volatile types");

	public:
		entity_view() noexcept = default;
		entity_view(T* ptr) noexcept;

		template<size_t I>
		component_reference<T> get() const noexcept;

		template<typename Ty>
		component_reference<Ty> get() const noexcept;

		entity_view<T>& operator++() noexcept;
		entity_view<T>& operator--() noexcept;

		bool operator==(const entity_view<T>& other) const noexcept;
		bool operator!=(const entity_view<T>& other) const noexcept;

	private:
		T* m_ptr = nullptr;
	};

	template<typename... T>
	class view_iterator {
		static_assert(is_unique_v<std::remove_const_t<T>...>, "View must only contain unique types");
		static_assert(
		    (std::is_same_v<std::remove_const_t<T>, std::remove_cvref_t<T>> && ...), "View must only contain non-reference and non-volatile types"
		);

	public:
		using value_type = entity_view<T...>;
		using pointer = const entity_view<T...>*;
		using reference = const entity_view<T...>&;
		using iterator_category = std::forward_iterator_tag;

	public:
		view_iterator() noexcept = default;
		view_iterator(archetype* const* begin, archetype* const* end) noexcept;

		reference operator*() const noexcept;
		pointer operator->() const noexcept;

		view_iterator<T...>& operator++();
		view_iterator<T...> operator++(int);

		bool operator==(const view_iterator<T...>& other) const noexcept;
		bool operator!=(const view_iterator<T...>& other) const noexcept;

	private:
		entity_view<T...> m_entity;
		size_t m_row = 0;
		archetype* const* m_archetype = nullptr;
		archetype* const* m_end = nullptr;
	};

	template<typename... T>
	class view {
		static_assert(is_unique_v<std::remove_const_t<T>...>, "View must only contain unique types");
		static_assert(
		    (std::is_same_v<std::remove_const_t<T>, std::remove_cvref_t<T>> && ...), "View must only contain non-reference and non-volatile types"
		);

	public:
		using iterator = view_iterator<T...>;

	public:
		view(archetype* begin, archetype* end) noexcept;

		iterator begin() const noexcept;
		iterator end() const noexcept;

	private:
		small_vector<archetype*, 8> m_archetypes;
	};

	template<typename T, typename... Rest>
	inline entity_view<T, Rest...>::entity_view(T* ptr, Rest*... rest) noexcept : m_ptr(ptr), m_rest(rest...) {}

	template<typename T, typename... Rest>
	template<size_t I>
	inline component_reference<std::tuple_element_t<I, entity_view<T, Rest...>>> entity_view<T, Rest...>::get() const noexcept {
		if constexpr(I == 0) {
			return *m_ptr;
		} else {
			return m_rest.template get<I - 1>();
		}
	}

	template<typename T, typename... Rest>
	template<typename Ty>
	inline component_reference<Ty> entity_view<T, Rest...>::get() const noexcept {
		if constexpr(std::is_same<T, Ty>()) {
			return *m_ptr;
		} else {
			return m_rest.template get<Ty>();
		}
	}

	template<typename T, typename... Rest>
	inline entity_view<T, Rest...>& entity_view<T, Rest...>::operator++() noexcept {
		++m_ptr;
		++m_rest;
		return *this;
	}

	template<typename T, typename... Rest>
	inline entity_view<T, Rest...>& entity_view<T, Rest...>::operator--() noexcept {
		--m_ptr;
		--m_rest;
		return *this;
	}

	template<typename T, typename... Rest>
	bool entity_view<T, Rest...>::operator==(const entity_view<T, Rest...>& other) const noexcept {
		return m_ptr == other.m_ptr; // Only comparing the address of the first component, which should be enough.
	}

	template<typename T, typename... Rest>
	bool entity_view<T, Rest...>::operator!=(const entity_view<T, Rest...>& other) const noexcept {
		return m_ptr != other.m_ptr; // Only comparing the address of the first component, which should be enough.
	}

	template<typename T>
	inline entity_view<T>::entity_view(T* ptr) noexcept : m_ptr(ptr) {}

	template<typename T>
	template<size_t I>
	inline component_reference<T> entity_view<T>::get() const noexcept {
		static_assert(I == 0, "Index out of bounds");
		return *m_ptr;
	}

	template<typename T>
	template<typename Ty>
	inline component_reference<Ty> entity_view<T>::get() const noexcept {
		static_assert(std::is_same_v<T, Ty>, "Entity view must contain requested type");
		return *m_ptr;
	}

	template<typename T>
	inline entity_view<T>& entity_view<T>::operator++() noexcept {
		++m_ptr;
		return *this;
	}

	template<typename T>
	inline entity_view<T>& entity_view<T>::operator--() noexcept {
		--m_ptr;
		return *this;
	}

	template<typename T>
	bool entity_view<T>::operator==(const entity_view<T>& other) const noexcept {
		return m_ptr == other.m_ptr; // Only comparing the address of the first component, which should be enough.
	}

	template<typename T>
	bool entity_view<T>::operator!=(const entity_view<T>& other) const noexcept {
		return m_ptr != other.m_ptr; // Only comparing the address of the first component, which should be enough.
	}

	template<typename... T>
	view_iterator<T...>::view_iterator(archetype* const* begin, archetype* const* end) noexcept : 
		m_archetype(begin),
		m_end(end)
	{
		if(m_archetype != m_end) m_entity = entity_view<T...>((*begin)->data<std::remove_const_t<T>>()...);
	}

	template<typename... T>
	inline view_iterator<T...>::reference view_iterator<T...>::operator*() const noexcept {
		return m_entity;
	}

	template<typename... T>
	view_iterator<T...>::pointer view_iterator<T...>::operator->() const noexcept {
		return &m_entity;
	}

	template<typename... T>
	view_iterator<T...>& view_iterator<T...>::operator++() {
		if(++m_row >= (*m_archetype)->size) {
			m_row = 0;
			if(++m_archetype == m_end)
				m_entity.m_ptr = nullptr;
		} else {
			++m_entity;
		}
		return *this;
	}

	template<typename... T>
	view_iterator<T...> view_iterator<T...>::operator++(int) {
		auto tmp = *this;
		++(*this);
		return tmp;
	}

	template<typename... T>
	bool view_iterator<T...>::operator==(const view_iterator<T...>& other) const noexcept {
		return m_entity == other.m_entity;
	}

	template<typename... T>
	bool view_iterator<T...>::operator!=(const view_iterator<T...>& other) const noexcept {
		return m_entity != other.m_entity;
	}

	template<typename... T>
	view<T...>::view(archetype* begin, archetype* end) noexcept {
		for(archetype* it = begin; it != end; ++it)
			if(it->contains<std::remove_const_t<T>...>())
				m_archetypes.push_back(it);
	}

	template<typename... T>
	view<T...>::iterator view<T...>::begin() const noexcept {
		return iterator(m_archetypes.begin(), m_archetypes.end());
	}

	template<typename... T>
	view<T...>::iterator view<T...>::end() const noexcept {
		return {};
	}

} // namespace ecs

namespace std {

	template<typename... T>
	struct tuple_size<ecs::entity_view<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};

	template<typename... T, size_t I>
	struct tuple_element<I, ecs::entity_view<T...>> {
		using type = std::tuple_element_t<I, std::tuple<T...>>;
	};

	// entity_views are views, so even if the view is const, the thing its pointing at could still be mutable
	template<typename... T, size_t I>
	struct tuple_element<I, const ecs::entity_view<T...>> {
		using type = std::tuple_element_t<I, std::tuple<T...>>;
	};

} // namespace std
