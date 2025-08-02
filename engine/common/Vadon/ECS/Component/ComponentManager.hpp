#ifndef VADON_ECS_COMPONENT_COMPONENTMANAGER_HPP
#define VADON_ECS_COMPONENT_COMPONENTMANAGER_HPP
#include <Vadon/ECS/Component/Query.hpp>
namespace Vadon::ECS
{
	class ComponentManager
	{
	public:
		~ComponentManager();

		VADONCOMMON_API void* add_component(EntityHandle entity, ComponentID type_id);
		VADONCOMMON_API bool has_component(EntityHandle entity, ComponentID type_id) const;
		VADONCOMMON_API void* get_component(EntityHandle entity, ComponentID type_id);
		VADONCOMMON_API void remove_component(EntityHandle entity, ComponentID type_id);

		template<typename T>
		T& add_component(EntityHandle entity)
		{
			TypedComponentPool<T>* typed_pool = get_component_pool<T>();
			T& new_component = typed_pool->typed_add_component(entity);

			return new_component;
		}

		template<typename T>
		bool has_component(EntityHandle entity) const
		{
			return has_component(entity, get_component_type_id<T>());
		}

		template<typename T>
		const T* get_component(EntityHandle entity) const
		{
			const TypedComponentPool<T>* typed_pool = find_component_pool<T>();
			if (typed_pool != nullptr)
			{
				return typed_pool->typed_get_component(entity);
			}
			return nullptr;
		}

		template<typename T>
		T* get_component(EntityHandle entity) { return const_cast<T*>(std::as_const(*this).get_component<T>(entity)); }

		template<typename ...Components> ComponentTuple<Components*...> get_component_tuple(EntityHandle entity)
		{			
			const ComponentIDArray<Components...>& component_ids = unpack_component_ids<Components...>();

			std::array<void*, sizeof...(Components)> component_pointers;

			internal_get_component_tuple(entity, component_ids, component_pointers);
			auto component_ptr_it = component_pointers.begin();

			return { (static_cast<Components*>(*(component_ptr_it++)))... };
		}

		template<typename ...Components> ComponentQuery<Components...> run_component_query() 
		{
			using Query = ComponentQuery<Components...>;
			typename Query::ComponentInfoArray info_array = { ComponentQueryBase::ComponentInfo{ .pool = find_component_pool<DecayedComponentType<Components>>(), .required = std::is_reference_v<Components> }... };

			return Query(info_array);
		}

		VADONCOMMON_API ComponentIDList get_component_list(EntityHandle entity) const;

		// FIXME: when removing, we do not check whether the component was there in the first place
		// might want to implement a "shortcut" to removing component once we already queried it
		template<typename T>
		void remove_component(EntityHandle entity)
		{
			TypedComponentPool<T>* typed_pool = find_component_pool<T>();
			if (typed_pool != nullptr)
			{
				typed_pool->remove_component(entity);
			}
		}

		template<typename T>
		static ComponentID get_component_type_id()
		{
			return ComponentPoolInterface::get_component_type_id<T>();
		}

		template<typename T>
		void set_entity_tag(EntityHandle entity, bool enabled)
		{
			static_assert(std::is_empty_v<T>, "Tag must be empty type!");
			if (enabled == true)
			{
				add_component<T>(entity);
			}
			else
			{
				remove_component<T>(entity);
			}
		}

		template<typename T>
		bool entity_has_tag(EntityHandle entity)
		{
			static_assert(std::is_empty_v<T>, "Tag must be empty type!");
			return has_component<T>(entity);
		}
	private:
		ComponentManager();

		void remove_entity(EntityHandle entity);
		void remove_entity_batch(const EntityList& entity_batch);

		template<typename T>
		const TypedComponentPool<T>* find_component_pool() const
		{
			const ComponentID component_type_id = get_component_type_id<T>();
			return static_cast<const TypedComponentPool<T>*>(find_component_pool(component_type_id));
		}

		template<typename T>
		TypedComponentPool<T>* find_component_pool() { return const_cast<TypedComponentPool<T>*>(std::as_const(*this).find_component_pool<T>()); }

		template<typename T>
		TypedComponentPool<T>* get_component_pool()
		{
			return static_cast<TypedComponentPool<T>*>(get_component_pool(get_component_type_id<T>()));
		}

		VADONCOMMON_API const ComponentPoolInterface* find_component_pool(ComponentID type_id) const;
		ComponentPoolInterface* find_component_pool(ComponentID type_id) { return const_cast<ComponentPoolInterface*>(std::as_const(*this).find_component_pool(type_id)); }

		VADONCOMMON_API ComponentPoolInterface* get_component_pool(ComponentID type_id);

		VADONCOMMON_API void internal_get_component_tuple(EntityHandle entity, ComponentIDSpan component_ids, ComponentSpan components);

		void clear();
		
		std::unordered_map<ComponentID, ComponentPoolInterface*> m_component_pools;

		friend class World;
	};
}
#endif