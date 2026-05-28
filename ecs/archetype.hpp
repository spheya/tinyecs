#pragma once

#include <cassert>
#include <cstdlib>
#include <type_traits>
#include <utility>

#include "meta.hpp"
#include "signature.hpp"

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

		[[nodiscard]] bool is_trivially_copyable_data() const noexcept;
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

		template<typename T>
		[[nodiscard]] T* data();

		template<typename T>
		[[nodiscard]] const T* data() const;

		template<typename... T>
		[[nodiscard]] bool contains() const;

	private:
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
		static_assert(!std::is_reference_v<T>, "Column element type cannot be a reference");

		if constexpr(std::is_trivially_copyable_v<T>) {
			destroy = nullptr;
		} else {
			destroy = [](const void* t) { static_cast<const T*>(t)->~T(); };
			mass_destroy = [](const void* t, size_t count) {
				for(size_t i = 0; i < count; ++i) static_cast<const T*>(t)[i].~T();
			};

			move = [](void* RESTRICT dst, void* RESTRICT src) { new(dst) T(std::move(*static_cast<T*>(src))); };
			mass_move = [](void* RESTRICT dst, void* RESTRICT src, size_t count) {
				for(size_t i = 0; i < count; ++i) new(static_cast<T*>(dst) + i) T(std::move(static_cast<T*>(src)[i]));
			};
		}

		element_size = sizeof(T);
		component_id = type_id<T>();

		data = static_cast<char*>(malloc(S * sizeof(T)));
	}

	inline bool column::is_trivially_copyable_data() const noexcept {
		return destroy == nullptr;
	}

	inline archetype::archetype(archetype&& other) noexcept :
	    signature(std::move(other.signature)), size(other.size), capacity(other.capacity), entities(other.entities), columns(other.columns) {
		other.entities = nullptr;
		other.columns = nullptr;
	}

	inline archetype& archetype::operator=(archetype&& other) noexcept {
		for(int i = 0; i < signature.size(); ++i) {
			columns[i].mass_destroy(columns[i].data, size);
			free(columns[i].data);
		}
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
		for(int i = 0; i < signature.size(); ++i) {
			if(!columns->is_trivially_copyable_data()) columns[i].mass_destroy(columns[i].data, size);
			free(columns[i].data);
		}
		free(entities);
		free(columns);
	}

	template<typename... T>
	inline void archetype::init() {
		static_assert(is_unique_v<T...>, "Archetype signature must only contain unique types");
		static_assert((std::is_same_v<T, std::remove_cvref_t<T>> && ...), "Archetype signature only allows non-reference, unqualified types");
		static_assert(!(std::is_same_v<T, entity> || ...), "Archetype signature cannot contain entity type");

		constexpr size_t initial_capacity = 8;
		columns = static_cast<column*>(malloc(sizeof...(T) * sizeof(column)));
		(columns[signature.index_of<T>()].template init<T, initial_capacity>(), ...);
		capacity = initial_capacity;
		entities = static_cast<entity*>(malloc(capacity * sizeof(entity)));
	}

	template<typename... T>
	inline size_t archetype::add_entity(entity entity, T&&... components) {
		static_assert(is_unique_v<std::remove_cvref_t<T>...>, "Archetype signature must only contain unique types");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, ecs::entity> || ...), "Archetype signature cannot contain entity type");
		assert(sizeof...(T) == signature.size());

		if(size + 1 > capacity) reallocate(capacity * 2);
		entities[size] = entity;
		(
		    [&] {
			    auto it = std::find_if(columns, columns + signature.size(), [](const column& c) {
				    return c.component_id == type_id<std::remove_cvref_t<T>>(); // todo: compare performance of this vs signature::index_of
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
			if(column.is_trivially_copyable_data()) {
			    memcpy(column.data + column.element_size * row, column.data + column.element_size * size, column.element_size);
			} else {
				column.destroy(column.data + column.element_size * row);
				column.move(column.data + column.element_size * row, column.data + column.element_size * size);
			}
		}
		return entities[row];
	}

	template<typename T>
	T* archetype::data() {
		if constexpr(std::is_same_v<std::remove_const_t<T>, entity>) {
			return entities;
		} else {
			return reinterpret_cast<T*>(columns[signature.index_of<std::remove_const_t<T>>()].data);
		}
	}

	template<typename T>
	const T* archetype::data() const {
		if constexpr(std::is_same_v<std::remove_const_t<T>, entity>) {
			return entities;
		} else {
			return reinterpret_cast<const T*>(columns[signature.index_of<std::remove_const_t<T>>()].data);
		}
	}

	template<typename... T>
	bool archetype::contains() const {
		return ([&]() {
			if constexpr(std::is_same_v<std::remove_const_t<T>, entity>) {
				return true;
			} else {
				return signature.contains<T>();
			}
		}() && ...);
	}

	inline void archetype::reallocate(size_t newCapacity) {
		entities = static_cast<entity*>(realloc(entities, newCapacity * sizeof(entity)));
		for(int i = 0; i < signature.size(); ++i) {
			column& column = columns[i];
			if(column.is_trivially_copyable_data()) {
				column.data = static_cast<char*>(realloc(column.data, newCapacity * column.element_size));
			} else {
				char* newData = static_cast<char*>(malloc(newCapacity * column.element_size)); // todo: padding, error handling
				column.mass_move(newData, column.data, size);
				free(column.data);
				column.data = newData;
			}
		}
		capacity = newCapacity;
	}

} // namespace ecs
