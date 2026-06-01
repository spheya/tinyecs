#pragma once

#include <cassert>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "archetype.hpp"
#include "meta.hpp"
#include "signature.hpp"

namespace ecs {

	template<typename... T>
	using entity_view = std::tuple<component_reference<T>...>;

	struct entity_record {
		size_type archetype;
		size_type row;
	};

	class world {
	public:
		template<typename... T>
		entity add_entity(T&&... components);

		void remove_entity(entity e);

		template<typename T>
		[[nodiscard]] bool has_component(entity e) const noexcept;

		template<typename... T>
		[[nodiscard]] bool has_components(entity e) const noexcept;

		template<typename T>
		[[nodiscard]] component_reference<T> get_component(entity e);

		template<typename T>
		[[nodiscard]] component_reference<const T> get_component(entity e) const;

		template<typename... T>
		[[nodiscard]] entity_view<T...> get_components(entity e);

		template<typename... T>
		[[nodiscard]] entity_view<const T...> get_components(entity e) const;

		template<typename Func>
		void each(Func&& func);

		template<typename Func>
		void each(Func&& func) const;

	public:
		entity nextEntity = null_entity + 1;
		std::unordered_map<entity, entity_record> entities;
		std::unordered_map<signature, size_t> archetype_lut;
		std::vector<archetype> archetypes;
	};

	namespace internal {
		template<typename Archetypes, typename Func, typename... Args>
		inline void each_impl(Archetypes& archetypes, Func&& func, function_args<Args...> /* args */) {
			static_assert(is_unique_v<std::remove_cvref_t<Args>...>, "Components must be unique");
			static_assert(!(std::is_same_v<std::remove_volatile_t<Args>, ecs::entity&> || ...), "Cannot get a mutable reference to entity");

			for(auto& archetype : archetypes) {
				auto base = std::make_tuple(archetype.template column<std::remove_cvref_t<Args>>()...);
				if(((std::get<decltype(archetype.template column<std::remove_cvref_t<Args>>())>(base) == nullptr) || ...)) continue;
				size_type size = archetype.size();
				for(size_type i = 0; i < size; ++i)
					std::forward<Func>(func)(std::get<decltype(archetype.template column<std::remove_cvref_t<Args>>())>(base)[i]...);
			}
		}
	} // namespace internal

	template<typename... T>
	inline entity world::add_entity(T&&... components) {
		signature sig = create_signature<std::remove_cvref_t<T>...>();

		archetype* archetype;
		size_type archetype_index;

		auto it = archetype_lut.find(sig);
		if(it == archetype_lut.end()) {
			archetype_index = archetypes.size();
			archetype_lut.emplace(sig, archetype_index);
			archetypes.emplace_back(std::move(sig));
			archetype = &archetypes.back();
			archetype->init<std::remove_cvref_t<T>...>();
		} else {
			archetype_index = it->second;
			archetype = &archetypes[archetype_index];
		}

		entity e = nextEntity++;
		size_type row = archetype->add_entity<std::remove_cvref_t<T>...>(e, std::forward<T>(components)...);
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
	inline bool world::has_component(entity e) const noexcept {
		return has_components<T>(e);
	}

	template<typename... T>
	inline bool world::has_components(entity e) const noexcept {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		return (archetype.column<T>() && ...);
	}

	template<typename T>
	inline component_reference<T> world::get_component(entity e) {
		return std::get<0>(get_components<T>(e));
	}

	template<typename T>
	inline component_reference<const T> world::get_component(entity e) const {
		return std::get<0>(get_components<T>(e));
	}

	template<typename... T>
	inline entity_view<T...> world::get_components(entity e) {
		entity_record record = entities.at(e);
		archetype& archetype = archetypes[record.archetype];
		std::tuple<T*...> columns(archetype.column<T>()...);
		assert(std::get<T*>(columns) && ...); // entity doesnt contain all components
		return entity_view<T...>(std::get<T*>(columns)[record.row]...);
	}

	template<typename... T>
	inline entity_view<const T...> world::get_components(entity e) const {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		std::tuple<const T*...> columns(archetype.column<const T>()...);
		assert(std::get<T*>(columns) && ...); // entity doesnt contain all components
		return entity_view<T...>(std::get<const T*>(columns)[record.row]...);
	}

	template<typename Func>
	inline void world::each(Func&& func) {
		internal::each_impl(archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

	template<typename Func>
	inline void world::each(Func&& func) const {
		internal::each_impl(archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

} // namespace ecs
