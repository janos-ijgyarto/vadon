#ifndef VADON_ECS_COMPONENT_POOL_HPP
#define VADON_ECS_COMPONENT_POOL_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <Vadon/ECS/Component/Event.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

#include <optional>
#include <vector>
namespace Vadon::ECS
{
	class ComponentPoolInterface
	{
	public:
		virtual ~ComponentPoolInterface() {}

		virtual void* add_component(EntityHandle entity) = 0; // NOTE: used for serialization

		virtual const void* get_component(EntityHandle entity) const = 0;
		void* get_component(EntityHandle entity) { return const_cast<void*>(std::as_const(*this).get_component(entity)); }

		bool has_component(EntityHandle entity) const { return get_component(entity) != nullptr; }

		virtual void remove_component(EntityHandle entity) = 0;

		virtual uint32_t get_active_count() const = 0;
		virtual EntityList get_entities() const = 0;

		template<typename T>
		static ComponentID get_component_type_id();

		virtual void clear() = 0;
	protected:
		VADONCOMMON_API void dispatch_component_event(const ComponentEvent& event);

		std::vector<ComponentEventCallback> m_event_callbacks;

		friend class ComponentManager;
	};

	template<typename T>
	class TypedComponentPool : public ComponentPoolInterface
	{
	public:
		// TODO: add some way to connect add/remove event listeners? Or should that be at a higher layer?
		using _Pool = TypedComponentPool<T>;
		virtual T& typed_add_component(EntityHandle entity) = 0;

		virtual const T* typed_get_component(EntityHandle entity) const = 0;
		const void* get_component(EntityHandle entity) const override { return typed_get_component(entity); }
		T* typed_get_component(EntityHandle entity) { return const_cast<T*>(std::as_const(*this).typed_get_component(entity)); }
	};

	class DefaultComponentPoolBase
	{
	protected:
		VADONCOMMON_API std::optional<EntityList::const_iterator> add_entity(EntityHandle entity);
		VADONCOMMON_API EntityList::const_iterator find_entity(EntityHandle entity) const;
		VADONCOMMON_API void remove_entity(EntityList::const_iterator entity_iterator);

		VADONCOMMON_API void default_pool_clear();

		EntityList m_entity_lookup;
		std::vector<uint32_t> m_component_offsets;
	};

	template<typename T>
	class DefaultComponentPool : public TypedComponentPool<T>, public DefaultComponentPoolBase
	{
	public:
		void* add_component(EntityHandle entity) override
		{
			// TODO: assert if we try to add component more than once?
			T& new_component = typed_add_component(entity);
			return &new_component;
		}

		T& typed_add_component(EntityHandle entity) override
		{
			auto entity_it = add_entity(entity);
			if (entity_it)
			{
				// Entity already present
				// FIXME: move to shared utility function!
				const uint32_t component_offset = m_component_offsets[std::distance(m_entity_lookup.cbegin(), entity_it.value())];
				return m_components[component_offset];
			}

			m_components.emplace_back();
			ComponentPoolInterface::dispatch_component_event(ComponentEvent{ .owner = entity, .type_id = ComponentPoolInterface::get_component_type_id<T>(), .event_type = ComponentEventType::ADDED });

			return m_components.back();
		}

		const T* typed_get_component(EntityHandle entity) const override
		{
			auto entity_it = find_entity(entity);
			if (entity_it != m_entity_lookup.end())
			{
				const uint32_t component_offset = m_component_offsets[std::distance(m_entity_lookup.begin(), entity_it)];
				return &m_components[component_offset];
			}

			return nullptr;
		}

		void remove_component(EntityHandle entity) override
		{
			auto entity_it = find_entity(entity);
			if (entity_it != m_entity_lookup.end())
			{
				ComponentPoolInterface::dispatch_component_event(ComponentEvent{ .owner = entity, .type_id = ComponentPoolInterface::get_component_type_id<T>(), .event_type = ComponentEventType::REMOVED });

				const uint32_t component_offset = m_component_offsets[std::distance(m_entity_lookup.cbegin(), entity_it)];
				auto removed_it = m_components.begin() + component_offset;
				auto back_it = m_components.end() - 1;
				if (removed_it != back_it)
				{
					*removed_it = std::move(m_components.back());
				}
				m_components.pop_back();
				remove_entity(entity_it);
			}
		}

		uint32_t get_active_count() const override
		{
			return static_cast<uint32_t>(m_components.size());
		}

		EntityList get_entities() const override
		{
			return m_entity_lookup;
		}

		void clear() override
		{
			DefaultComponentPoolBase::default_pool_clear();
			m_components.clear();
		}
	private:
		std::vector<T> m_components;
	};

	// NOTE: Client code can specialize as needed
	template<typename T>
	class ComponentPool : public DefaultComponentPool<T>
	{};

	template<typename T>
	ComponentID ComponentPoolInterface::get_component_type_id()
	{
		static_assert(std::is_base_of_v<TypedComponentPool<T>, ComponentPool<T>>, "Error in Vadon::ECS: component type must be derived from Vadon::ECS::TypedComponentPool<T>!");
		return Vadon::Utilities::TypeRegistry::get_type_id<T>();
	}
}
#endif