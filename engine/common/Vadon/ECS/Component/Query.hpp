#ifndef VADON_ECS_COMPONENT_QUERY_HPP
#define VADON_ECS_COMPONENT_QUERY_HPP
#include <Vadon/ECS/Component/Pool.hpp>
#include <array>
#include <algorithm>
namespace Vadon::ECS
{
	using ComponentIDSpan = std::span<const ComponentID>;

	template<typename ...Components>
	using ComponentIDArray = std::array<ComponentID, sizeof...(Components)>;

	template<typename ...Components>
	const ComponentIDArray<Components...>& unpack_component_ids()
	{
		static ComponentIDArray<Components...> component_ids = { ComponentPoolInterface::get_component_type_id<Components>()... };
		return component_ids;
	}

	template<typename ...Components>
	const ComponentIDArray<Components...>& unpack_component_ids_sorted()
	{
		static ComponentIDArray<Components...> sorted_ids = (
			+[]() 
			{
				ComponentIDArray<Components...> unpacked_ids = unpack_component_ids<Components...>();
				std::sort(unpacked_ids.begin(), unpacked_ids.end());
				return unpacked_ids;
			}
		)();

		return sorted_ids;
	}

	template<typename ...Components>
	bool is_component_tuple_unique()
	{
		const ComponentIDArray<Components...> component_ids = unpack_component_ids_sorted<Components...>();

		for (size_t current_index = 1; current_index < component_ids.size(); ++current_index)
		{
			if (component_ids[current_index] == component_ids[current_index - 1])
			{
				return false;
			}
		}

		return true;
	}

	template <typename T>
	using DecayedComponentType = std::remove_reference_t<std::remove_pointer_t<std::decay_t<T>>>;

	class ComponentQueryBase
	{
	protected:
		struct ComponentInfo
		{
			ComponentPoolInterface* pool;
			bool required;
		};

		class IteratorBase
		{
		public:
			VADONCOMMON_API bool is_valid() const;
			VADONCOMMON_API EntityHandle get_entity() const;
			VADONCOMMON_API void next();
		protected:
			VADONCOMMON_API IteratorBase(ComponentQueryBase& query);
			void advance();

			ComponentManager& get_manager() const { return m_query.m_manager; }

			EntityList& get_entity_list() { return m_query.m_entities; }
			const EntityList& get_entity_list() const { return m_query.m_entities; }

			ComponentQueryBase& m_query;
			size_t m_offset;
		};

		ComponentQueryBase(ComponentManager& manager)
			: m_manager(manager)
		{ }

		VADONCOMMON_API void initialize(std::span<ComponentInfo> component_info);
		bool check_components(size_t index);

		ComponentManager& m_manager;
		EntityList m_entities;
		std::span<ComponentInfo> m_component_info;
	};

	template<typename ...Components>
	class ComponentQuery : public ComponentQueryBase
	{
	public:
		using _Query = ComponentQuery<Components...>;

		class Iterator : public ComponentQueryBase::IteratorBase
		{
		private:
			template<typename T>
			static constexpr bool contains_type() 
			{
				return (std::is_same_v<T, DecayedComponentType<Components>> || ...);
			}
		public:
			template<typename T>
			TypedComponentHandle<T> get_component() const
			{
				static_assert(contains_type<T>() == true, "Type not in query!");
				return get_manager().get_component<T>(get_entity_list()[m_offset]);
			}
		private:
			Iterator(_Query& query) : ComponentQueryBase::IteratorBase(query)
			{
			}

			friend _Query;
		};

		Iterator get_iterator() { return Iterator(*this); }

	private:
		using ComponentInfoArray = std::array<ComponentInfo, sizeof...(Components)>;

		ComponentQuery(ComponentManager& manager, ComponentInfoArray component_info)
			: ComponentQueryBase(manager)
			, m_component_info_array(component_info)
		{		
			initialize(m_component_info_array);
		}

		ComponentInfoArray m_component_info_array;
		friend ComponentManager;
	};

}
#endif