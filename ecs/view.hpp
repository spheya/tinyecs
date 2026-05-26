#pragma once

#include <tuple>
#include <type_traits>

#include "ecs/archetype.hpp"

namespace ecs {

	template<typename... T>
	class entity_view;

	template<typename T, typename... Rest>
	class entity_view<T, Rest...> {
		template<typename... Ty>
		friend class view_iterator;

	public:
		entity_view() noexcept = default;
		entity_view(T* ptr, Rest*... rest) noexcept;

		template<size_t I>
		std::tuple_element_t<I, entity_view<T, Rest...>>& get() const noexcept;

		// todo: get by type

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
	public:
		entity_view() noexcept = default;
		entity_view(T* ptr) noexcept;

		template<size_t I>
		T& get() const noexcept;

		entity_view<T>& operator++() noexcept;
		entity_view<T>& operator--() noexcept;

		bool operator==(const entity_view<T>& other) const noexcept;
		bool operator!=(const entity_view<T>& other) const noexcept;

	private:
		T* m_ptr = nullptr;
	};

	template<typename... T>
	class view_iterator {
	public:
		using value_type = entity_view<T...>;
		using pointer = const entity_view<T...>*;
		using reference = const entity_view<T...>&;
		using iterator_category = std::forward_iterator_tag;

	public:
		view_iterator() noexcept = default;
		view_iterator(const archetype* begin, const archetype* end) noexcept;

		reference operator*() const noexcept;
		pointer operator->() const noexcept;

		view_iterator<T...>& operator++();
		view_iterator<T...> operator++(int);

		bool operator==(const view_iterator<T...>& other) const noexcept;
		bool operator!=(const view_iterator<T...>& other) const noexcept;

	private:
		const archetype* m_archetype = nullptr;
		const archetype* m_archetype_end = nullptr;
		entity_view<T...> m_entity;
		size_t m_row = 0;
	};

	template<typename... T>
	class view {
	public:
		using iterator = view_iterator<T...>;

	public:
		view(const archetype* begin, const archetype* end) noexcept;

		iterator begin() const noexcept;
		iterator end() const noexcept;

	private:
		const archetype* m_begin;
		const archetype* m_end;
	};

	template<typename T, typename... Rest>
	inline entity_view<T, Rest...>::entity_view(T* ptr, Rest*... rest) noexcept : m_ptr(ptr), m_rest(rest...) {}

	template<typename T, typename... Rest>
	template<size_t I>
	inline std::tuple_element_t<I, entity_view<T, Rest...>>& entity_view<T, Rest...>::get() const noexcept {
		if constexpr(I == 0) {
			return *m_ptr;
		} else {
			return m_rest.template get<I - 1>();
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
	inline T& entity_view<T>::get() const noexcept {
		static_assert(I == 0, "index out of range");
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
	view_iterator<T...>::view_iterator(const archetype* begin, const archetype* end) noexcept : m_archetype(begin), m_archetype_end(end) {
		// todo: avoid duplicate code
		// todo: make an ecs::view pre-calculate the begin and end archetype
		while(!m_archetype->signature.contains<std::remove_cvref_t<T>...>()) {
			if(++m_archetype == m_archetype_end) {
				m_entity.m_ptr = nullptr;
				return;
			}
		}
		m_entity = entity_view<T...>(reinterpret_cast<T*>(m_archetype->columns[m_archetype->signature.getIndex(type_id<std::remove_cvref_t<T>>())].data)...);
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
		if(++m_row >= m_archetype->size) {
			m_row = 0;
			do {
				if(++m_archetype == m_archetype_end) {
					m_entity.m_ptr = nullptr;
					return *this;
				}
			} while(!m_archetype->signature.contains<std::remove_cvref_t<T>...>());
			m_entity = entity_view<T...>(reinterpret_cast<T*>(m_archetype->columns[m_archetype->signature.getIndex(type_id<std::remove_cvref_t<T>>())].data)...);
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
		return m_entity == other.m_entity; // Only comparing the address of the first component, which should be enough.
	}

	template<typename... T>
	bool view_iterator<T...>::operator!=(const view_iterator<T...>& other) const noexcept {
		return m_entity != other.m_entity; // Only comparing the address of the first component, which should be enough.
	}

	template<typename... T>
	view<T...>::view(const archetype* begin, const archetype* end) noexcept : m_begin(begin), m_end(end) {}

	template<typename... T>
	view<T...>::iterator view<T...>::begin() const noexcept {
		return iterator(m_begin, m_end);
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

} // namespace std
