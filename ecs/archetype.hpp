#pragma once

#include <cassert>
#include <cstdlib>
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
		void init() {
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
	};

	class archetype {
	public:
		archetype(signature signature) : signature(std::move(signature)) {}

		template<typename... T>
		void init() {
			columns = new column[sizeof...(T)];
			init_impl<T...>(std::index_sequence_for<T...>());
		}

		template<typename... T>
		size_t add_entity(entity entity, T&&... components) {
			if(size + 1 > capacity) reallocate(capacity * 2);
			entities[size] = entity;
			(
			    [&] {
				    auto it = std::find_if(columns, columns + signature.size(), [](const column& c) { return c.component_id == type_id<T>(); });
					assert(it != columns + signature.size());
					reinterpret_cast<T*>(it->data)[size] = std::forward<T>(components);
			    }(),
			    ...
			);
			return size++;
		}

	private:
		template<typename... T, size_t... I>
		void init_impl(std::index_sequence<I...> /* seq */) {
			constexpr size_t initial_capacity = 8;
			(columns[I].init<T, initial_capacity>(), ...);
			capacity = initial_capacity;
			entities = static_cast<entity*>(malloc(capacity * sizeof(entity)));
		}

		void reallocate(size_t newCapacity) {
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

	public:
		signature signature;
		size_t size = 0;
		size_t capacity = 0;
		entity* entities = nullptr;
		column* columns = nullptr;
	};

} // namespace ecs
