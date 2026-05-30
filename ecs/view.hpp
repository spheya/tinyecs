#pragma once

#include <tuple>
#include <type_traits>

#include "archetype.hpp"
#include "meta.hpp"
#include "small_vector.hpp"

namespace ecs {

	template<typename... T>
	using entity_view = std::tuple<component_reference<T>...>;

	template<typename... T>
	class view_iterator {
		static_assert(is_unique_v<std::remove_const_t<T>...>, "View must only contain unique types");
		static_assert(
		    (std::is_same_v<std::remove_const_t<T>, std::remove_cvref_t<T>> && ...), "View must only contain non-reference and non-volatile types"
		);

	public:
		using value_type = std::tuple<component_reference<T>...>;
		using iterator_category = std::forward_iterator_tag;

	public:
		view_iterator() noexcept = default;
		view_iterator(const std::pair<archetype*, std::tuple<T*...>>* begin, const std::pair<archetype*, std::tuple<T*...>>* end) noexcept;

		value_type operator*() const noexcept;

		view_iterator<T...>& operator++();
		view_iterator<T...> operator++(int);

		bool operator==(const view_iterator<T...>& other) const noexcept;
		bool operator!=(const view_iterator<T...>& other) const noexcept;

	private:
		size_type m_row = 0;
		const std::pair<archetype*, std::tuple<T*...>>* m_archetype = nullptr;
		const std::pair<archetype*, std::tuple<T*...>>* m_end = nullptr;
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
		small_vector<std::pair<archetype*, std::tuple<T*...>>, 8> m_archetypes;
	};

	template<typename... T>
	view_iterator<T...>::view_iterator(const std::pair<archetype*, std::tuple<T*...>>* begin, const std::pair<archetype*, std::tuple<T*...>>* end) noexcept :
	    m_archetype(begin), m_end(end) {}

	template<typename... T>
	inline view_iterator<T...>::value_type view_iterator<T...>::operator*() const noexcept {
		return value_type(*(std::get<T*>(m_archetype->second) + m_row)...);
	}

	template<typename... T>
	view_iterator<T...>& view_iterator<T...>::operator++() {
		if(++m_row >= m_archetype->first->size) [[unlikely]] {
			m_row = 0;
			++m_archetype;
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
		return m_archetype == other.m_archetype && m_row == other.m_row;
	}

	template<typename... T>
	bool view_iterator<T...>::operator!=(const view_iterator<T...>& other) const noexcept {
		return m_archetype != other.m_archetype || m_row != other.m_row;
	}

	template<typename... T>
	view<T...>::view(archetype* begin, archetype* end) noexcept {
		for(archetype* it = begin; it != end; ++it)
			if(it->size != 0 && it->contains<std::remove_const_t<T>...>()) m_archetypes.emplace_back(it, std::tuple<T*...>(it->data<std::remove_const_t<T>>()...));
	}

	template<typename... T>
	view<T...>::iterator view<T...>::begin() const noexcept {
		return iterator(m_archetypes.begin(), m_archetypes.end());
	}

	template<typename... T>
	view<T...>::iterator view<T...>::end() const noexcept {
		return iterator(m_archetypes.end(), m_archetypes.end());
	}

} // namespace ecs
