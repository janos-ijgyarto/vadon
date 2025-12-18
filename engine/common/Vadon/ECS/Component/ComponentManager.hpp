#ifndef VADON_ECS_COMPONENT_COMPONENTMANAGER_HPP
#define VADON_ECS_COMPONENT_COMPONENTMANAGER_HPP
#include <Vadon/ECS/Component/Query.hpp>
namespace Vadon::ECS
{
	class ComponentManager
	{
	public:
		~ComponentManager();

		VADONCOMMON_API ComponentHandle add_component(EntityHandle entity, ComponentID type_id);
		VADONCOMMON_API bool has_component(EntityHandle entity, ComponentID type_id) const;
		VADONCOMMON_API ComponentHandle get_component(EntityHandle entity, ComponentID type_id) const;
		VADONCOMMON_API void remove_component(EntityHandle entity, ComponentID type_id);

		template<typename T>
		TypedComponentHandle<T> add_component(EntityHandle entity)
		{
			TypedComponentPool<T>* typed_pool = get_component_pool<T>();
			typed_pool->typed_add_component(entity);

			return TypedComponentHandle<T>(typed_pool, entity);
		}

		template<typename T>
		bool has_component(EntityHandle entity) const
		{
			return has_component(entity, get_component_type_id<T>());
		}

		template<typename T>
		TypedComponentHandle<T> get_component(EntityHandle entity) const
		{
			TypedComponentPool<T>* typed_pool = find_component_pool<T>();
			return TypedComponentHandle<T>(typed_pool, entity);
		}

		template<typename ...Components>
		ComponentQuery<Components...> run_component_query()
		{
			using Query = ComponentQuery<Components...>;
			typename Query::ComponentInfoArray info_array = { ComponentQueryBase::ComponentInfo{.pool = find_component_pool<DecayedComponentType<Components>>(), .required = std::is_reference_v<Components> }... };

			return Query(*this, info_array);
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
		TypedComponentPool<T>* find_component_pool() const
		{
			const ComponentID component_type_id = get_component_type_id<T>();
			return static_cast<TypedComponentPool<T>*>(find_component_pool(component_type_id));
		}
				
		template<typename T>
		TypedComponentPool<T>* get_component_pool()
		{
			return static_cast<TypedComponentPool<T>*>(get_component_pool(get_component_type_id<T>()));
		}

		VADONCOMMON_API ComponentPoolInterface* find_component_pool(ComponentID type_id) const;

		VADONCOMMON_API ComponentPoolInterface* get_component_pool(ComponentID type_id);

		void clear();

		std::unordered_map<ComponentID, ComponentPoolInterface*> m_component_pools;

		friend class World;
	};

	template<typename T>
	const T& TypedComponentHandle<T>::operator*() const 
	{
		const TypedComponentPool<T>* typed_pool = static_cast<const TypedComponentPool<T>*>(m_pool);
		return *typed_pool->typed_get_component(m_owner);
	}

	template<typename T>
	const T* TypedComponentHandle<T>::operator->() const 
	{ 
		if (m_pool != nullptr)
		{
			const TypedComponentPool<T>* typed_pool = static_cast<const TypedComponentPool<T>*>(m_pool);
			return typed_pool->typed_get_component(m_owner);
		}
		return nullptr;
	}
}
#endif