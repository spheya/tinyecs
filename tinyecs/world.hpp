#pragma once

#include <functional>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "archetype.hpp" // IWYU pragma: export
#include "meta.hpp"      // IWYU pragma: export
#include "signature.hpp" // IWYU pragma: export

namespace tinyecs {

	struct entity_record {
		size_type archetype;
		size_type row;
	};

	class world {
	public:
		template<typename... T>
		entity create_entity(T&&... components);

		void remove_entity(entity e);

		template<typename... T>
		void add(entity e, T&&... components);

		template<typename... T>
		[[nodiscard]] bool has(entity e) const noexcept;

		template<typename... T>
		[[nodiscard]] bool has_any(entity e) const noexcept;

		template<typename... T>
		[[nodiscard]] component_pack_t<component_reference<T>...> get(entity e);

		template<typename... T>
		[[nodiscard]] component_pack_t<component_reference<const T>...> get(entity e) const;

		template<typename... T>
		[[nodiscard]] component_pack_t<T*...> try_get(entity e);

		template<typename... T>
		[[nodiscard]] component_pack_t<const T*...> try_get(entity e) const;

		template<typename Func>
		void each(Func&& func);

		template<typename Func>
		void each(Func&& func) const;

	private:
		entity nextEntity = null_entity + 1;
		std::unordered_map<entity, entity_record> entities;
		std::unordered_map<signature, size_type> archetype_lut;
		std::vector<archetype> archetypes;
	};

	// todo: clean this up
	namespace internal {
		template<typename T>
		T* archetype_column_ptr(type_index type_idx, archetype& archetype, bool& success) {
			TINYECS_ASSUME(type_id<T>() == type_idx);
			if constexpr(std::is_same_v<T, entity>) {
				return archetype.entities();
			} else {
				T* column = static_cast<T*>(archetype.find_column(type_idx));
				if(column == nullptr) success = false;
				return column;
			}
		}

		template<typename T>
		const T* archetype_column_ptr(type_index type_idx, const archetype& archetype, bool& success) {
			TINYECS_ASSUME(type_id<T>() == type_idx);
			if constexpr(std::is_same_v<T, entity>) {
				return archetype.entities();
			} else {
				const T* column = static_cast<const T*>(archetype.find_column(type_idx));
				if(column == nullptr) success = false;
				return column;
			}
		}

		template<typename... T, size_t... I, typename Archetype>
		auto get_base(type_index* indices, Archetype& archetype, bool& success, std::index_sequence<I...> /* seq */) {
			return std::make_tuple(archetype_column_ptr<T>(indices[I], archetype, success)...);
		}

		template<typename Archetypes, typename Func, typename... Args>
		inline void
		each_impl(Archetypes& archetypes, Func&& func, function_args<Args...> /* args */) { // NOLINT(cppcoreguidelines-missing-std-forward)
			static_assert(is_unique_v<std::remove_cvref_t<Args>...>, "Components must be unique");
			static_assert(!(std::is_same_v<std::remove_volatile_t<Args>, tinyecs::entity&> || ...), "Cannot get a mutable reference to entity");

			type_index indices[] = { type_id<std::remove_cvref_t<Args>>()... };

			for(auto& archetype : archetypes) {
				bool success = true;
				auto base = get_base<std::remove_cvref_t<Args>...>(indices, archetype, success, std::index_sequence_for<Args...>());
				if(!success) continue;
				size_type size = archetype.size();
				for(size_type i = 0; i < size; ++i) func(std::get<decltype(archetype.template find_column<std::remove_cvref_t<Args>>())>(base)[i]...);
			}
		}
	} // namespace internal

	template<typename... T>
	inline entity world::create_entity(T&&... components) {
		signature signature = create_signature<std::remove_cvref_t<T>...>();

		archetype* archetype;
		size_type archetype_index;

		auto it = archetype_lut.find(signature);
		if(it == archetype_lut.end()) {
			archetype_index = archetypes.size();
			archetype_lut.emplace(signature, archetype_index);
			archetypes.emplace_back(std::move(signature));
			archetype = &archetypes.back();
			archetype->init<std::remove_cvref_t<T>...>();
		} else {
			archetype_index = it->second;
			archetype = &archetypes[archetype_index];
		}

		entity e = nextEntity++;
		size_type row = archetype->add_entity(e, std::forward<T>(components)...);
		entities.emplace(e, entity_record{ .archetype = archetype_index, .row = row });
		return e;
	}

	inline void world::remove_entity(entity e) {
		entity_record record = entities.at(e);
		entity replacement = archetypes[record.archetype].remove_entity(record.row);
		if(replacement) entities.at(replacement).row = record.row;
		entities.erase(e);
	}

	template<typename... T>
	void world::add(entity e, T&&... components) {
		entity_record& record = entities.at(e);

		signature signature = extend_signature<std::remove_cvref_t<T>...>(archetypes[record.archetype].get_signature());

		archetype* dst_archetype;
		size_type dst_archetype_index;

		auto it = archetype_lut.find(signature);
		if(it == archetype_lut.end()) {
			dst_archetype_index = archetypes.size();
			archetype_lut.emplace(signature, dst_archetype_index);
			archetypes.emplace_back(archetypes[record.archetype].extend<std::remove_cvref_t<T>...>(std::move(signature)));
			dst_archetype = &archetypes.back();
		} else {
			dst_archetype_index = it->second;
			dst_archetype = &archetypes[dst_archetype_index];
		}

		size_type row = record.row;
		entity replacement = archetypes[record.archetype].move_entity(record.row, *dst_archetype, std::forward<T>(components)...);
		record.archetype = dst_archetype_index;
		if(replacement) entities.at(replacement).row = row;
	}

	template<typename... T>
	inline bool world::has(entity e) const noexcept {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		return (archetype.has_column<T>() && ...);
	}

	template<typename... T>
	[[nodiscard]] bool world::has_any(entity e) const noexcept {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		return (archetype.has_column<T>() || ...);
	}

	template<typename... T>
	inline component_pack_t<component_reference<T>...> world::get(entity e) {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		if constexpr(sizeof...(T) == 1) {
			return *try_get<T...>(e);
		} else {
			component_pack_t<T*...> components = try_get<T...>(e);
			// TINYECS_ASSUME(std::get<T*>(components) && ...); // entity does not contain components
			return component_pack_t<component_reference<T>...>(*std::get<T*>(components)...);
		}
	}

	template<typename... T>
	inline component_pack_t<component_reference<const T>...> world::get(entity e) const {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		if constexpr(sizeof...(T) == 1) {
			return *try_get<const T...>(e);
		} else {
			component_pack_t<const T*...> components = try_get<const T...>(e);
			// TINYECS_ASSUME(std::get<const T*>(components) && ...); // entity does not contain components
			return component_pack_t<component_reference<const T>...>(*std::get<const T*>(components)...);
		}
	}

	template<typename... T>
	inline component_pack_t<T*...> world::try_get(entity e) {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = entities.at(e);
		archetype& archetype = archetypes[record.archetype];
		std::tuple<T*...> columns(archetype.find_column<T>()...);
		if constexpr(sizeof...(T) == 1) {
			return std::get<0>(columns) + record.row;
		} else {
			return component_pack_t<T*...>(std::get<T*>(columns) + record.row...);
		}
	}

	template<typename... T>
	inline component_pack_t<const T*...> world::try_get(entity e) const {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = entities.at(e);
		const archetype& archetype = archetypes[record.archetype];
		std::tuple<T*...> columns(archetype.find_column<T>()...);
		if constexpr(sizeof...(T) == 1) {
			return std::get<0>(columns) + record.row;
		} else {
			return component_pack_t<const T*...>(std::get<const T*>(columns) + record.row...);
		}
	}

	template<typename Func>
	inline void world::each(Func&& func) {
		internal::each_impl(archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

	template<typename Func>
	inline void world::each(Func&& func) const {
		internal::each_impl(archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

} // namespace tinyecs
