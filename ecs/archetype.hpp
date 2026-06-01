#pragma once

#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "component.hpp"
#include "meta.hpp"
#include "signature.hpp"
#include "small_vector.hpp"

namespace ecs {

	class archetype {
	public:
		archetype(signature signature);
		archetype(archetype&) = delete;
		archetype& operator=(archetype&) = delete;
		archetype(archetype&& other) noexcept;
		archetype& operator=(archetype&& other) noexcept;
		~archetype();

	public:
		template<typename... T>
		void init();

		// returns the row that the entity lives on
		template<typename... T>
		size_type add_entity(entity entity, T&&... components);

		// returns the entity that is now stored on the row, or null_entity if the row is no longer used
		entity remove_entity(size_type row);

		template<typename T>
		[[nodiscard]] T* column() noexcept;

		template<typename T>
		[[nodiscard]] const T* column() const noexcept;

		[[nodiscard]] size_type size() const noexcept;

		void reserve(size_type capacity);

	private:
		template<typename... T, size_type... I>
		void init_impl(std::integer_sequence<size_type, I...> /* seq */);

		template<typename... T, size_type... I>
		void add_entity_impl(std::integer_sequence<size_type, I...> /* seq */);

	private:
		constexpr static size_t initial_capacity = 8;

		signature m_signature;
		size_type m_size = 0;
		size_type m_capacity = initial_capacity;
		entity* m_entities = nullptr;
		small_vector<void*, 8> m_columns;
		small_vector<component_ops, 1> m_component_ops;
	};

	inline archetype::archetype(signature signature) : m_signature(std::move(signature)) {}

	inline archetype::archetype(archetype&& other) noexcept :
	    m_signature(std::move(other.m_signature)),
	    m_size(other.m_size),
	    m_capacity(other.m_capacity),
	    m_entities(other.m_entities),
	    m_columns(std::move(other.m_columns)),
	    m_component_ops(std::move(other.m_component_ops)) {
		other.m_entities = nullptr;
	}

	inline archetype& archetype::operator=(archetype&& other) noexcept {
		if(this == &other) return *this;

		if(m_entities) {
			for(size_type i = 0; i < m_columns.size(); ++i) {
				const component_ops& ops = m_component_ops[i];
				char* column = static_cast<char*>(m_columns[i]);
				if(!ops.is_trivially_destructible()) ops.mass_destroy(column, m_size);
				free(column);
			}
			free(m_entities);
		}

		m_signature = std::move(other.m_signature);
		m_size = other.m_size;
		m_capacity = other.m_capacity;
		m_entities = other.m_entities;
		m_columns = std::move(other.m_columns);
		m_component_ops = std::move(other.m_component_ops);

		other.m_entities = nullptr;

		return *this;
	}

	inline archetype::~archetype() {
		if(m_entities) {
			for(size_type i = 0; i < m_columns.size(); ++i) {
				const component_ops& ops = m_component_ops[i];
				char* column = static_cast<char*>(m_columns[i]);
				if(!ops.is_trivially_destructible()) ops.mass_destroy(column, m_size);
				free(column);
			}
			free(m_entities);
		}
	}

	template<typename... T, size_type... I>
	inline void archetype::init_impl(std::integer_sequence<size_type, I...> /* seq */) {
		((m_columns[I] = malloc(initial_capacity * sizeof(T))), ...);
		if(!(m_columns[I] && ...)) throw std::bad_alloc();
	}

	template<typename... T>
	inline void archetype::init() {
		m_columns.resize(sizeof...(T));
		m_component_ops = { create_component_operations<T>()... };
		m_entities = static_cast<entity*>(malloc(initial_capacity * sizeof(entity)));
		if(!m_entities) throw std::bad_alloc();
		init_impl<T...>(std::make_index_sequence<sizeof...(T)>());
	}

	template<typename... T>
	inline size_type archetype::add_entity(entity entity, T&&... components) {
		if(m_size == m_capacity) reserve(m_capacity * 2);
		assert(column<std::remove_cvref_t<T>>() && ...); // archetype does not contain these components
		m_entities[m_size] = entity;
		(std::construct_at(column<std::remove_cvref_t<T>>() + m_size, std::forward<T>(components)), ...);
		return m_size++;
	}

	inline entity archetype::remove_entity(size_type row) {
		assert(row < m_size);
		--m_size;

		for(size_type i = 0; i < m_columns.size(); ++i) {
			const component_ops& ops = m_component_ops[i];
			char* column = static_cast<char*>(m_columns[i]);

			if(ops.is_trivially_copyable()) {
				if(row != m_size) memcpy(column + row * ops.component_size, column + m_size, ops.component_size);
			} else {
				if(!ops.is_trivially_destructible()) ops.destroy(column + row * ops.component_size);
				if(row != m_size) ops.relocate(column + row * ops.component_size, column + m_size * ops.component_size);
			}
		}

		if(row == m_size) return null_entity;
		m_entities[row] = m_entities[m_size];
		return m_entities[row];
	}

	template<typename T>
	inline T* archetype::column() noexcept {
		if constexpr(std::is_same_v<std::remove_const_t<T>, entity>) {
			return m_entities;
		} else {
			const component_id* it =
			    std::ranges::find(m_signature.components.begin(), m_signature.components.end(), type_id<std::remove_const_t<T>>());
			if(it == m_signature.components.end()) return nullptr;
			return reinterpret_cast<T*>(m_columns[it - m_signature.components.begin()]);
		}
	}

	template<typename T>
	inline const T* archetype::column() const noexcept {
		if constexpr(std::is_same_v<std::remove_const_t<T>, entity>) {
			return m_entities;
		} else {
			const component_id* it =
			    std::ranges::find(m_signature.components.begin(), m_signature.components.end(), type_id<std::remove_const_t<T>>());
			if(it == m_signature.components.end()) return nullptr;
			return reinterpret_cast<const T*>(m_columns[it - m_signature.components.begin()]);
		}
	}

	inline size_type archetype::size() const noexcept {
		return m_size;
	}

	inline void archetype::reserve(size_type capacity) {
		assert(capacity > m_size);

		auto* new_entities = static_cast<entity*>(realloc(m_entities, capacity * sizeof(entity)));
		if(!new_entities) throw std::bad_alloc();
		m_entities = new_entities;

		for(size_type i = 0; i < m_columns.size(); ++i) {
			const component_ops& ops = m_component_ops[i];
			char* column = static_cast<char*>(m_columns[i]);

			if(ops.is_trivially_copyable()) {
				void* new_column = realloc(m_columns[i], capacity * ops.component_size);
				if(!new_column) throw std::bad_alloc();
				m_columns[i] = new_column;
			} else {
				char* new_column = static_cast<char*>(malloc(capacity * ops.component_size));
				if(!new_column) throw std::bad_alloc();
				ops.mass_relocate(new_column, column, m_size);
				free(column);
				m_columns[i] = new_column;
			}
		}

		m_capacity = capacity;
	}

} // namespace ecs
