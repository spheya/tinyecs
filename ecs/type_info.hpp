#pragma once

#include <cstdint>
#include <atomic>

namespace ecs {

	using type_index = uint32_t;

	namespace internal {
		[[nodiscard]] inline type_index next_type_index() noexcept {
			static std::atomic<type_index> idx = 0;
			return idx.fetch_add(1, std::memory_order_relaxed);
		}
	}

	template<typename T>
	[[nodiscard]] inline type_index type_id() noexcept {
		static type_index idx = internal::next_type_index();
		return idx;
	}

}
