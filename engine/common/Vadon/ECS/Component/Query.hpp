#ifndef VADON_ECS_COMPONENT_QUERY_HPP
#define VADON_ECS_COMPONENT_QUERY_HPP
#include <Vadon/ECS/Component/Pool.hpp>
#include <array>
#include <algorithm>
namespace Vadon::ECS
{
	using ComponentSpan = std::span<void*>;

	using ComponentIDSpan = std::span<const ComponentID>;

	template<typename ...Components>
	using ComponentTuple = std::tuple<Components...>;

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
			VADONCOMMON_API IteratorBase(ComponentQueryBase& query, ComponentSpan components);
			void advance();

			template<typename T> static DecayedComponentType<T>* as_component(void* component_pointer) { return static_cast<DecayedComponentType<T>*>(component_pointer); }

			ComponentQueryBase& m_query;
			size_t m_offset;
			ComponentSpan m_components;
		};

		ComponentQueryBase() = default;

		VADONCOMMON_API void initialize(std::span<ComponentInfo> component_info);
		bool get_components(size_t index, ComponentSpan components);

		EntityList m_entities;
		std::span<ComponentInfo> m_component_info;
	};

	template<typename ...Components>
	class ComponentQuery : public ComponentQueryBase
	{
	public:
		using _Query = ComponentQuery<Components...>;
		using Tuple = ComponentTuple<Components...>;

		class Iterator : public ComponentQueryBase::IteratorBase
		{
		public:
			ComponentTuple<Components...> get_tuple() { return internal_get_tuple(); }
		private:
			using ComponentPointerArray = std::array<void*, sizeof...(Components)>;

			Iterator(_Query& query)
				: ComponentQueryBase::IteratorBase(query, m_component_array)
			{

			}

			ComponentTuple<Components...> internal_get_tuple()
			{
				// Use fold expression to iterate through our iterator array and assemble the tuple
				auto component_it = m_component_array.begin();
				return { get_tuple_element<Components>(component_it++) ... };
			}

			template<typename T>
			T get_tuple_element(typename ComponentPointerArray::iterator&& array_it)
			{
				auto* element_ptr = as_component<T>(*array_it);
				if constexpr (std::is_pointer_v<T>)
				{
					return element_ptr;
				}
				else
				{
					return *element_ptr;
				}
			}

			ComponentPointerArray m_component_array;

			friend _Query;
		};

		Iterator get_iterator() { return Iterator(*this); }

	private:
		using ComponentInfoArray = std::array<ComponentInfo, sizeof...(Components)>;

		ComponentQuery(ComponentInfoArray component_info)
			: m_component_info_array(component_info)
		{		
			initialize(m_component_info_array);
		}

		ComponentInfoArray m_component_info_array;
		friend class ComponentManager;
	};

}
#endif