#pragma once

#include <cstdint>

#include "type_info.hpp"

namespace ecs {

	using entity = uint32_t;
	using component_id = type_index;

	constexpr entity null_entity = entity(0);

} // namespace ecs
