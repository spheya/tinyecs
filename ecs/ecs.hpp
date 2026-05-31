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
		size_t archetype;
		size_t row;
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
		[[nodiscard]] reference<T> get_component(entity e);

		template<typename T>
		[[nodiscard]] reference<const T> get_component(entity e) const;

		template<typename... T>
		[[nodiscard]] entity_view<T...> get_components(entity e);

		template<typename... T>
		[[nodiscard]] entity_view<const T...> get_components(entity e) const;

		template<typename Func>
		void each(Func&& func);

	private:
		template<typename Func, typename... Args>
		void each_impl(Func&& func, function_args<Args...> /* args */);

	public:
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
			archetype->init<std::remove_cvref_t<T>...>();
		} else {
			archetype_index = it->second;
			archetype = &archetypes[archetype_index];
		}

		entity e = nextEntity++;
		size_t row = archetype->add_entity<std::remove_cvref_t<T>...>(e, std::forward<T>(components)...);
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
		return archetype.signature.contains<T...>();
	}

	template<typename T>
	inline reference<T> world::get_component(entity e) {
		entity_record record = entities.at(e);
		archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains(type_id<std::remove_const_t<T>>()));
		return archetype.data<std::remove_const_t<T>>()[record.row];
	}

	template<typename T>
	inline reference<const T> world::get_component(entity e) const {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains(type_id<std::remove_const_t<T>>()));
		return archetype.data<std::remove_const_t<T>>()[record.row];
	}

	template<typename... T>
	inline entity_view<T...> world::get_components(entity e) {
		entity_record record = entities.at(e);
		archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains<T...>());
		return entity_view<T...>(archetype.data<std::remove_const_t<T>>()[record.row]...);
	}

	template<typename... T>
	inline entity_view<const T...> world::get_components(entity e) const {
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		assert(archetype.signature.contains<std::remove_const_t<T>...>());
		return entity_view<const T...>(archetype.data<std::remove_const_t<T>>()[record.row]...);
	}


	template<typename Func>
	void world::each(Func&& func) {
		each_impl(std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

	template<typename Func, typename... Args>
	void world::each_impl(Func&& func, function_args<Args...> /* args */) {
		for(archetype& archetype : archetypes) {
			if(!archetype.contains<std::remove_cvref_t<Args>...>()) continue;
			std::tuple<std::remove_cvref_t<Args>*...> base(archetype.data<std::remove_cvref_t<Args>>()...);

			for(size_type i = 0; i < archetype.size; ++i)
				std::forward<Func>(func)(*(std::get<std::remove_cvref_t<Args>*>(base) + i)...);
		}
	}

} // namespace ecs
