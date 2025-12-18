#ifndef VADON_ECS_COMPONENT_POOL_HPP
#define VADON_ECS_COMPONENT_POOL_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

#include <optional>
#include <unordered_set>
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

		virtual void clear() = 0;

		template<typename T>
		static ComponentID get_component_type_id()
		{
			return Vadon::Utilities::TypeRegistry::get_type_id<T>();
		}
	protected:
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
		static constexpr uint32_t c_invalid_component_index = static_cast<uint32_t>(-1);

		VADONCOMMON_API bool add_entity(EntityHandle entity);
		VADONCOMMON_API uint32_t get_component_index(EntityHandle entity) const;
		VADONCOMMON_API uint32_t remove_entity(EntityHandle entity);

		VADONCOMMON_API void default_pool_clear();

		EntityList m_reverse_lookup;
		std::vector<uint32_t> m_sparse_lookup;
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
			if (add_entity(entity) == false)
			{
				// Entity already present
				return m_components[m_sparse_lookup[entity.handle.index]];
			}

			m_components.emplace_back();			
			return m_components.back();
		}

		const T* typed_get_component(EntityHandle entity) const override
		{
			const uint32_t component_index = get_component_index(entity);
			if (component_index != c_invalid_component_index)
			{
				return &m_components[component_index];
			}

			return nullptr;
		}

		void remove_component(EntityHandle entity) override
		{
			const uint32_t component_index = remove_entity(entity);
			if (component_index != c_invalid_component_index)
			{
				if (component_index != (m_components.size() - 1))
				{
					m_components[component_index] = std::move(m_components.back());
				}
				m_components.pop_back();
			}
		}

		uint32_t get_active_count() const override
		{
			return static_cast<uint32_t>(m_components.size());
		}

		EntityList get_entities() const override
		{
			return m_reverse_lookup;
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

	class TagPoolBase
	{
	protected:
		VADONCOMMON_API bool add_entity(EntityHandle entity);
		VADONCOMMON_API bool has_entity(EntityHandle entity) const;
		VADONCOMMON_API void remove_entity(EntityHandle entity);

		VADONCOMMON_API void tag_pool_clear();

		VADONCOMMON_API EntityList get_tagged_entity_list() const;

		std::unordered_set<uint64_t> m_entity_lookup;
	};

	template<typename T>
	class TagPool : public TypedComponentPool<T>, public TagPoolBase
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
			const bool is_added = add_entity(entity);
			if (is_added == false)
			{
				// TODO: report error if entity already tagged?
			}

			return m_prototype;
		}

		const T* typed_get_component(EntityHandle entity) const override
		{
			if (has_entity(entity) == true)
			{
				return &m_prototype;
			}

			return nullptr;
		}

		void remove_component(EntityHandle entity) override
		{
			remove_entity(entity);
		}

		uint32_t get_active_count() const override
		{
			return static_cast<uint32_t>(m_entity_lookup.size());
		}

		EntityList get_entities() const override
		{
			return get_tagged_entity_list();
		}

		void clear() override
		{
			tag_pool_clear();
		}
	private:
		T m_prototype;
	};
}
#endif