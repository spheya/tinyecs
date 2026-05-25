#pragma once

#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include "macros.hpp"
#include "signature.hpp"
#include "type_info.hpp"
#include "types.hpp"

namespace ecs {

	struct column {
		void (*destroy)(const void* ptr);
		void (*mass_destroy)(const void* ptr, size_t count);
		void (*move)(void* RESTRICT dst, void* RESTRICT src);
		void (*mass_move)(void* RESTRICT dst, void* RESTRICT src, size_t count);

		size_t element_size;
		component_id component_id;
		char* data;

		template<typename T, size_t S>
		void init();
	};

	class archetype {
	public:
		archetype(signature signature) : signature(std::move(signature)) {}
		archetype(archetype&) = delete;
		archetype& operator=(archetype&) = delete;
		archetype(archetype&& other) noexcept;
		archetype& operator=(archetype&& other) noexcept;
		~archetype();

		template<typename... T>
		void init();

		template<typename... T>
		size_t add_entity(entity entity, T&&... components);

		// returns the entity that took its place or `null_entity` if the archetype is empty
		entity remove_entity(size_t row);

	private:
		template<typename... T, size_t... I>
		void init_impl(std::index_sequence<I...> /* seq */);

		void reallocate(size_t newCapacity);

	public:
		signature signature;
		size_t size = 0;
		size_t capacity = 0;
		entity* entities = nullptr;
		column* columns = nullptr;
	};

	template<typename T, size_t S>
	inline void column::init() {
		destroy = [](const void* t) { static_cast<const T*>(t)->~T(); };
		mass_destroy = [](const void* t, size_t count) {
			for(size_t i = 0; i < count; ++i) static_cast<const T*>(t)[i].~T();
		};

		move = [](void* RESTRICT dst, void* RESTRICT src) { new(dst) T(std::move(*static_cast<T*>(src))); };
		mass_move = [](void* RESTRICT dst, void* RESTRICT src, size_t count) {
			for(size_t i = 0; i < count; ++i) new(static_cast<T*>(dst) + i) T(std::move(static_cast<T*>(src)[i]));
		};

		element_size = sizeof(T);
		component_id = type_id<T>();

		data = static_cast<char*>(malloc(S * sizeof(T)));
	}

	inline archetype::archetype(archetype&& other) noexcept :
	    signature(std::move(other.signature)), size(other.size), capacity(other.capacity), entities(other.entities), columns(other.columns) {
		other.entities = nullptr;
		other.columns = nullptr;
	}

	inline archetype& archetype::operator=(archetype&& other) noexcept {
		for(int i = 0; i < signature.size(); ++i) free(columns[i].data);
		free(entities);
		delete[] columns;

		signature = std::move(other.signature);
		size = other.size;
		capacity = other.capacity;
		entities = other.entities;
		columns = other.columns;

		other.entities = nullptr;
		other.columns = nullptr;

		return *this;
	}

	inline archetype::~archetype() {
		for(int i = 0; i < signature.size(); ++i) free(columns[i].data);
		free(entities);
		delete[] columns;
	}

	template<typename... T>
	inline void archetype::init() {
		columns = new column[sizeof...(T)];
		init_impl<std::remove_cvref_t<T>...>(std::index_sequence_for<T...>());
	}

	template<typename... T>
	inline size_t archetype::add_entity(entity entity, T&&... components) {
		if(size + 1 > capacity) reallocate(capacity * 2);
		entities[size] = entity;
		(
		    [&] {
			    auto it = std::find_if(columns, columns + signature.size(), [](const column& c) {
				    return c.component_id == type_id<std::remove_cvref_t<T>>();
			    });
			    assert(it != columns + signature.size());
			    reinterpret_cast<std::remove_cvref_t<T>*>(it->data)[size] = std::forward<T>(components);
		    }(),
		    ...
		);
		return size++;
	}

	// returns the entity that took its place or `null_entity` if the archetype is empty
	inline entity archetype::remove_entity(size_t row) {
		assert(row < size);
		--size;
		if(size == 0) return null_entity;
		entities[row] = entities[size];
		for(int i = 0; i < signature.size(); ++i) {
			column& column = columns[i];
			column.destroy(column.data + column.element_size * row);
			column.move(column.data + column.element_size * row, column.data + column.element_size * size);
		}
		return entities[row];
	}

	template<typename... T, size_t... I>
	inline void archetype::init_impl(std::index_sequence<I...> /* seq */) {
		constexpr size_t initial_capacity = 8;
		(columns[I].init<T, initial_capacity>(), ...);
		capacity = initial_capacity;
		entities = static_cast<entity*>(malloc(capacity * sizeof(entity)));
	}

	inline void archetype::reallocate(size_t newCapacity) {
		realloc(entities, newCapacity);
		for(int i = 0; i < signature.size(); ++i) {
			column& column = columns[i];
			char* newData = reinterpret_cast<char*>(malloc(newCapacity * column.element_size)); // todo: padding, error handling
			column.mass_move(newData, column.data, size);
			free(column.data);
			column.data = newData;
		}
		capacity = newCapacity;
	}

} // namespace ecs
