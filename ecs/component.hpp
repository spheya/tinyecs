#pragma once

#include <memory>
#include <type_traits>

#include "meta.hpp"

namespace ecs {

	struct component_ops {
		void (*destroy)(void* ptr);
		void (*mass_destroy)(void* ptr, size_type count);
		void (*move_construct)(void* RESTRICT dst, void* RESTRICT src);
		void (*mass_move_construct)(void* RESTRICT dst, void* RESTRICT src, size_type count);

		[[nodiscard]] bool is_trivially_copyable() const;
		[[nodiscard]] bool is_trivially_destructible() const;
	};

	template<typename T>
	component_ops create_component_operations() {
		component_ops result{};
		if constexpr(!std::is_trivially_copyable_v<T>) {
			result.move_construct = [](void* RESTRICT dst, void* RESTRICT src) {
				std::construct_at(static_cast<T*>(dst), std::move(*static_cast<T*>(src)));
			};
			result.mass_move_construct = [](void* RESTRICT dst, void* RESTRICT src, size_type count) {
				for(size_type i = 0; i < count; ++i) std::construct_at(static_cast<T*>(dst) + i, std::move(static_cast<T*>(src)[i]));
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

	inline bool component_ops::is_trivially_copyable() const {
		return move_construct == nullptr;
	}

	inline bool component_ops::is_trivially_destructible() const {
		return destroy == nullptr;
	}

} // namespace ecs
