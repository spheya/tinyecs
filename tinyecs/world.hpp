#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "archetype.hpp"
#include "meta.hpp"
#include "reflection.hpp"
#include "signature.hpp"
#include "small_vector.hpp"
#include "tinyecs/meta.hpp"

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
		void set(entity e, T&&... components);

		template<typename... T>
		void remove(entity e);

		[[nodiscard]] size_type component_count(entity e) const noexcept;

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

		void visit(entity e, void* user_data);

		template<typename... T>
		void clear();

	private:
		template<typename T>
		void init_component_reflection();

		template<typename... T>
		std::pair<archetype*, size_type> get_or_create_archetype();

		template<typename... T>
		std::pair<archetype*, size_type> get_or_extend_archetype(const archetype& base);

		template<typename... T>
		std::pair<archetype*, size_type> get_or_reduce_archetype(const archetype& base);

		entity m_nextEntity = null_entity + 1;
		std::unordered_map<entity, entity_record> m_entities;
		std::unordered_map<signature, size_type> m_archetype_lut;
		std::unordered_map<component_id, void (*)(void*, void*)> m_component_reflection;
		std::vector<archetype> m_archetypes;
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
		auto&& [archetype, archetype_index] = get_or_create_archetype<std::remove_cvref_t<T>...>();

		entity e = m_nextEntity++;
		size_type row = archetype->add_entity(e);
		archetype->init_entity(row, std::forward<T>(components)...);
		m_entities.emplace(e, entity_record{ .archetype = archetype_index, .row = row });
		return e;
	}

	inline void world::remove_entity(entity e) {
		entity_record record = m_entities.at(e);
		entity replacement = m_archetypes[record.archetype].remove_entity(record.row);
		if(replacement) m_entities.at(replacement).row = record.row;
		m_entities.erase(e);
	}

	template<typename... T>
	inline void world::add(entity e, T&&... components) {
		static_assert(sizeof...(T) > 0, "Must add at least one component");

		entity_record& record = m_entities.at(e);
		auto&& [dst_archetype, dst_archetype_index] = get_or_extend_archetype<std::remove_cvref_t<T>...>(m_archetypes[record.archetype]);
		archetype& src_archetype = m_archetypes[record.archetype];

		TINYECS_ASSUME(dst_archetype != &src_archetype);                          // ... what?
		TINYECS_ASSUME(!src_archetype.contains<std::remove_cvref_t<T>>() && ...); // cannot contain duplicate components

		size_type new_row = dst_archetype->add_entity(e);
		dst_archetype->init_entity(new_row, std::forward<T>(components)...);
		entity replacement = src_archetype.move_entity(new_row, record.row, *dst_archetype);
		if(replacement) m_entities.at(replacement).row = record.row;
		record.archetype = dst_archetype_index;
		record.row = new_row;
	}

	template<typename... T>
	inline void world::set(entity e, T&&... components) {
		static_assert(sizeof...(T) > 0, "Must set at least one component");

		entity_record& record = m_entities.at(e);
		if((m_archetypes[record.archetype].contains<std::remove_cvref_t<T>>() && ...)) {
			// fast path: current archetype supports components
			archetype& archetype = m_archetypes[record.archetype];
			(std::destroy_at(archetype.column<std::remove_cvref_t<T>>() + record.row), ...);
			(std::construct_at(archetype.column<std::remove_cvref_t<T>>() + record.row, std::forward<T>(components)), ...);
		} else {
			// slow path: entity needs to be moved to another arche
			auto&& [dst_archetype, dst_archetype_index] = get_or_extend_archetype<std::remove_cvref_t<T>...>(m_archetypes[record.archetype]);
			archetype& src_archetype = m_archetypes[record.archetype];

			TINYECS_ASSUME(dst_archetype != &src_archetype); // ... what?

			size_type new_row = dst_archetype->add_entity(e);
			// todo: moving the entity will also move around components that are gonna be overriden during init_entity, we need a way to skip those
			entity replacement = src_archetype.move_entity(new_row, record.row, *dst_archetype);
			dst_archetype->init_entity(new_row, std::forward<T>(components)...);
			if(replacement) m_entities.at(replacement).row = record.row;
			record.archetype = dst_archetype_index;
			record.row = new_row;
		}
	}

	template<typename... T>
	inline void world::remove(entity e) {
		static_assert(sizeof...(T) > 0, "Must remove at least one component");

		entity_record& record = m_entities.at(e);
		auto&& [dst_archetype, dst_archetype_index] = get_or_reduce_archetype<std::remove_cvref_t<T>...>(m_archetypes[record.archetype]);
		archetype& src_archetype = m_archetypes[record.archetype];

		TINYECS_ASSUME(dst_archetype != &src_archetype);                         // ... what?
		TINYECS_ASSUME(src_archetype.contains<std::remove_cvref_t<T>>() && ...); // entity doesnt have these components

		size_type new_row = dst_archetype->add_entity(e);
		entity replacement = src_archetype.move_entity(new_row, record.row, *dst_archetype);
		if(replacement) m_entities.at(replacement).row = record.row;
		record.archetype = dst_archetype_index;
		record.row = new_row;
	}

	inline size_type world::component_count(entity e) const noexcept {
		entity_record record = m_entities.at(e);
		return m_archetypes[record.archetype].get_signature().components.size();
	}

	template<typename... T>
	inline bool world::has(entity e) const noexcept {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		entity_record record = m_entities.at(e);
		const archetype& archetype = m_archetypes[record.archetype];
		return (archetype.contains<T>() && ...);
	}

	template<typename... T>
	inline bool world::has_any(entity e) const noexcept {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = m_entities.at(e);
		const archetype& archetype = m_archetypes[record.archetype];
		return (archetype.contains<T>() || ...);
	}

	template<typename... T>
	inline component_pack_t<component_reference<T>...> world::get(entity e) {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = m_entities.at(e);
		archetype& archetype = m_archetypes[record.archetype];
		std::tuple<T*...> columns(archetype.column<T>()...);
		if constexpr(sizeof...(T) == 1) {
			return std::get<0>(columns)[record.row];
		} else {
			return component_pack_t<component_reference<T>...>(std::get<T*>(columns)[record.row]...);
		}
	}

	template<typename... T>
	inline component_pack_t<component_reference<const T>...> world::get(entity e) const {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = m_entities.at(e);
		const archetype& archetype = m_archetypes[record.archetype];
		std::tuple<const T*...> columns(archetype.column<T>()...);
		if constexpr(sizeof...(T) == 1) {
			return std::get<0>(columns)[record.row];
		} else {
			return component_pack_t<component_reference<const T>...>(std::get<T*>(columns)[record.row]...);
		}
	}

	template<typename... T>
	inline component_pack_t<T*...> world::try_get(entity e) {
		static_assert(sizeof...(T) != 0, "Needs at least one component");
		static_assert(!(std::is_same_v<std::remove_cvref_t<T>, entity> || ...), "An entity is an invalid component");
		entity_record record = m_entities.at(e);
		archetype& archetype = m_archetypes[record.archetype];
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
		entity_record record = m_entities.at(e);
		const archetype& archetype = m_archetypes[record.archetype];
		std::tuple<const T*...> columns(archetype.find_column<T>()...);
		if constexpr(sizeof...(T) == 1) {
			return std::get<0>(columns) + record.row;
		} else {
			return component_pack_t<const T*...>(std::get<const T*>(columns) + record.row...);
		}
	}

	template<typename Func>
	inline void world::each(Func&& func) {
		internal::each_impl(m_archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

	template<typename Func>
	inline void world::each(Func&& func) const {
		internal::each_impl(m_archetypes, std::forward<Func>(func), typename function_traits<Func>::arguments());
	}

	inline void world::visit(entity e, void* user_data) {
		entity_record record = m_entities.at(e);
		archetype& archetype = m_archetypes[record.archetype];
		for(component_id component : archetype.get_signature().components) {
			m_component_reflection.at(component)(user_data, archetype.component(component, record.row));
		}
	}

	template<typename T>
	inline void world::init_component_reflection() {
		m_component_reflection.emplace(
		    type_id<T>(), +[](void* user_data, void* component) { visit_component<T>{}(user_data, *static_cast<T*>(component)); }
		);
	}

	template<typename... T>
	inline void world::clear() {
		if constexpr(sizeof...(T) == 0) {
			// Clear everything
			for(archetype& a : m_archetypes) a.clear();
			m_entities.clear();
			m_nextEntity = null_entity + 1;
		} else {
			// Only clear archetypes with these components
			for(archetype& a : m_archetypes) {
				if(a.contains<T...>()) {
					for(auto* it = a.entities(); it != a.entities() + a.size(); ++it) m_entities.erase(*it);
					a.clear();
				}
			}
		}
	}

	// todo: remove duplicate logic in all get_or_*_archetype functions
	template<typename... T>
	inline std::pair<archetype*, size_type> world::get_or_create_archetype() {
		signature signature = make_signature<T...>();

		archetype* archetype;
		size_type archetype_index;

		auto it = m_archetype_lut.find(signature);
		if(it == m_archetype_lut.end()) {
			(init_component_reflection<T>(), ...);
			archetype_index = m_archetypes.size();
			m_archetype_lut.emplace(signature, archetype_index);
			m_archetypes.emplace_back(std::move(signature));
			archetype = &m_archetypes.back();
			archetype->init<T...>();
		} else {
			archetype_index = it->second;
			archetype = &m_archetypes[archetype_index];
		}

		return std::make_pair(archetype, archetype_index);
	}

	template<typename... T>
	inline std::pair<archetype*, size_type> world::get_or_extend_archetype(const archetype& base) {
		signature signature = extend_signature<T...>(base.get_signature());

		archetype* archetype;
		size_type archetype_index;

		auto it = m_archetype_lut.find(signature);
		if(it == m_archetype_lut.end()) {
			(init_component_reflection<T>(), ...);
			archetype_index = m_archetypes.size();
			m_archetype_lut.emplace(signature, archetype_index);
			m_archetypes.emplace_back(base.extend<T...>(std::move(signature)));
			archetype = &m_archetypes.back();
		} else {
			archetype_index = it->second;
			archetype = &m_archetypes[archetype_index];
		}

		return std::make_pair(archetype, archetype_index);
	}

	template<typename... T>
	inline std::pair<archetype*, size_type> world::get_or_reduce_archetype(const archetype& base) {
		signature signature = reduce_signature<T...>(base.get_signature());

		archetype* archetype;
		size_type archetype_index;

		auto it = m_archetype_lut.find(signature);
		if(it == m_archetype_lut.end()) {
			(init_component_reflection<T>(), ...);
			archetype_index = m_archetypes.size();
			m_archetype_lut.emplace(signature, archetype_index);
			m_archetypes.emplace_back(base.reduce<T...>(std::move(signature)));
			archetype = &m_archetypes.back();
		} else {
			archetype_index = it->second;
			archetype = &m_archetypes[archetype_index];
		}

		return std::make_pair(archetype, archetype_index);
	}

} // namespace tinyecs
