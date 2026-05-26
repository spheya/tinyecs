#pragma once

#include <cassert>
#include <type_traits>
#include <unordered_map>

#include "archetype.hpp"
#include "signature.hpp"
#include "types.hpp"
#include "view.hpp"

namespace ecs {

	struct entity_record {
		size_t archetype;
		size_t row;
	};

	class world {
	public:
		template<typename... T>
		entity add_entity(T&&... components);

		void remove_entity(entity e);

		template<typename T>
		[[nodiscard]] bool has_component(entity e) const;

		template<typename T>
		[[nodiscard]] T& get_component(entity e);

		template<typename T>
		[[nodiscard]] const T& get_component(entity e) const;

		template<typename... T>
		[[nodiscard]] entity_view<T...> get_components(entity e);

		template<typename... T>
		[[nodiscard]] view<T...> view();

		entity nextEntity = null_entity + 1;
		std::unordered_map<entity, entity_record> entities;
		std::unordered_map<signature, size_t> archetype_lut;
		std::vector<archetype> archetypes;
	};

	template<typename... T>
	inline entity world::add_entity(T&&... components) {
		signature sig = create_signature<std::remove_cvref_t<T>...>();

		archetype* archetype;
		size_t archetype_index;

		auto it = archetype_lut.find(sig);
		if(it == archetype_lut.end()) {
			archetype_index = archetypes.size();
			archetype_lut.emplace(sig, archetype_index);
			archetypes.emplace_back(std::move(sig));
			archetype = &archetypes.back();
			archetype->init<T...>();
		} else {
			archetype_index = it->second;
			archetype = &archetypes[archetype_index];
		}

		entity e = nextEntity++;
		size_t row = archetype->add_entity<T...>(e, std::forward<T>(components)...);
		entities.emplace(e, entity_record{ .archetype = archetype_index, .row = row });
		return e;
	}

	inline void world::remove_entity(entity e) {
		entity_record record = entities.at(e);
		entity replacement = archetypes[record.archetype].remove_entity(record.row);
		if(replacement) entities.at(replacement).row = record.row;
		entities.erase(e);
	}

	template<typename T>
	inline bool world::has_component(entity e) const {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		return archetype.signature.contains(type_id<T>());
	}

	template<typename T>
	inline T& world::get_component(entity e) {
		entity_record record = entities.at(e);
		archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains(type_id<T>()));
		return reinterpret_cast<T*>(archetype.columns[archetype.signature.getIndex(type_id<T>())].data)[record.row];
	}

	template<typename T>
	inline const T& world::get_component(entity e) const {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains(type_id<T>()));
		return reinterpret_cast<T*>(archetype.columns[archetype.signature.getIndex(type_id<T>())].data)[record.row];
	}

	template<typename... T>
	inline entity_view<T...> world::get_components(entity e) {
    	entity_record record = entities.at(e);
    	const archetype& archetype = archetypes[record.archetype];
        assert(archetype.signature.contains<T...>());
        return entity_view<T...>(reinterpret_cast<T*>(archetype.columns[archetype.signature.getIndex(type_id<T>())].data) + record.row...);
	}

	template<typename... T>
	inline view<T...> world::view() {
	    return ecs::view<T...>(archetypes.data(), archetypes.data() + archetypes.size());
	}

} // namespace ecs
