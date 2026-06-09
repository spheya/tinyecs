#pragma once

namespace tinyecs {

	template<typename... T>
	struct visit_component {
		void operator()([[maybe_unused]] void* user_data, [[maybe_unused]] T&... component) {}
	};

} // namespace tinyecs
