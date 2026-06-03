#pragma once

#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "meta.hpp"
#include "signature.hpp"
#include "small_vector.hpp"

namespace ecs {

	struct component_ops {
		void (*destroy)(void* ptr);
		void (*mass_destroy)(void* ptr, size_type count);
		void (*relocate)(void* dst, void* src);
		void (*mass_relocate)(void* dst, void* src, size_type count);
		size_t component_size;

		[[nodiscard]] bool is_trivially_copyable() const noexcept;
		[[nodiscard]] bool is_trivially_destructible() const noexcept;
	};

	class archetype {
	public:
		archetype(signature signature);
		archetype(archetype&) = delete;
		archetype& operator=(archetype&) = delete;
		archetype(archetype&& other) noexcept;
		archetype& operator=(archetype&& other) noexcept;
		~archetype();

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
		constexpr static size_t initial_capacity = 8;

		signature m_signature;
		size_type m_size = 0;
		size_type m_capacity = initial_capacity;
		entity* m_entities = nullptr;
		small_vector<void*, 8> m_columns;
		small_vector<component_ops, 1> m_component_ops;
	};

	template<typename T>
	component_ops create_component_operations() {
		component_ops result{};

		result.component_size = sizeof(T);

		if constexpr(!std::is_trivially_copyable_v<T>) {
			result.relocate = [](void* dst, void* src) {
				std::construct_at(static_cast<T*>(dst), std::move(*static_cast<T*>(src)));
				std::destroy_at(static_cast<T*>(src));
			};
			result.mass_relocate = [](void* dst, void* src, size_type count) {
				for(size_type i = 0; i < count; ++i) {
					std::construct_at(static_cast<T*>(dst) + i, std::move(static_cast<T*>(src)[i]));
					std::destroy_at(static_cast<T*>(src) + i);
				}
			};

			if constexpr(!std::is_trivially_destructible_v<T>) {
				result.destroy = [](void* ptr) { std::destroy_at(static_cast<T*>(ptr)); };
				result.mass_destroy = [](void* ptr, size_type count) {
					for(size_type i = 0; i < count; ++i) std::destroy_at(static_cast<T*>(ptr) + i);
				};
			}
		}
		return result;
	}

	inline bool component_ops::is_trivially_copyable() const noexcept {
		return relocate == nullptr;
	}

	inline bool component_ops::is_trivially_destructible() const noexcept {
		return destroy == nullptr;
	}

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

	template<typename... T>
	inline void archetype::init() {
		TINYECS_ASSUME(m_signature.components.size() == sizeof...(T));
		m_columns.resize(sizeof...(T));
		m_component_ops.resize(sizeof...(T));
		m_entities = static_cast<entity*>(malloc(initial_capacity * sizeof(entity)));
		if(!m_entities) throw std::bad_alloc();

		(
		    [&]() {
			    const component_id* it =
			        std::ranges::find(m_signature.components.begin(), m_signature.components.end(), type_id<std::remove_cvref_t<T>>());
			    TINYECS_ASSUME(it != m_signature.components.end());
			    auto index = size_type(it - m_signature.components.begin());
			    m_columns[index] = malloc(initial_capacity * sizeof(T));
			    m_component_ops[index] = create_component_operations<T>();
			    if(!m_columns[index]) throw std::bad_alloc();
		    }(),
		    ...
		);
	}

	template<typename... T>
	inline size_type archetype::add_entity(entity entity, T&&... components) {
		if(m_size == m_capacity) reserve(m_capacity * 2);
		TINYECS_ASSUME(column<std::remove_cvref_t<T>>() && ...); // archetype does not contain these components
		m_entities[m_size] = entity;
		(std::construct_at(column<std::remove_cvref_t<T>>() + m_size, std::forward<T>(components)), ...);
		return m_size++;
	}

	inline entity archetype::remove_entity(size_type row) {
		TINYECS_ASSUME(row < m_size);
		--m_size;

		for(size_type i = 0; i < m_columns.size(); ++i) {
			const component_ops& ops = m_component_ops[i];
			char* column = static_cast<char*>(m_columns[i]);

			if(ops.is_trivially_copyable()) {
				if(row != m_size) memcpy(column + row * ops.component_size, column + m_size * ops.component_size, ops.component_size);
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
			return reinterpret_cast<T*>(m_columns[size_type(it - m_signature.components.begin())]);
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
			return reinterpret_cast<const T*>(m_columns[size_type(it - m_signature.components.begin())]);
		}
	}

	inline size_type archetype::size() const noexcept {
		return m_size;
	}

	inline void archetype::reserve(size_type capacity) {
		TINYECS_ASSUME(capacity > m_size);

		auto* new_entities = static_cast<entity*>(realloc(m_entities, capacity * sizeof(entity)));
		if(!new_entities) throw std::bad_alloc();
		m_entities = new_entities;

		for(size_type i = 0; i < m_columns.size(); ++i) {
			const component_ops& ops = m_component_ops[i];
			char* column = static_cast<char*>(m_columns[i]);
			void* new_column;

			if(ops.is_trivially_copyable()) {
				new_column = realloc(column, capacity * ops.component_size);
			} else {
				new_column = malloc(capacity * ops.component_size);
				ops.mass_relocate(new_column, column, m_size);
				free(column);
			}

			if(!new_column) throw std::bad_alloc();
			m_columns[i] = new_column;
		}

		m_capacity = capacity;
	}

} // namespace ecs
