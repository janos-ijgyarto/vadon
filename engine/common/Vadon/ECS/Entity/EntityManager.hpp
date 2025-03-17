#ifndef VADON_ECS_ENTITY_ENTITYMANAGER_HPP
#define VADON_ECS_ENTITY_ENTITYMANAGER_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/Utilities/Container/ObjectPool/Pool.hpp>
namespace Vadon::ECS
{
	class EntityManager
	{
	public:
		VADONCOMMON_API EntityHandle create_entity();
		bool is_entity_valid(EntityHandle entity_handle) const { m_entity_pool.is_handle_valid(entity_handle); }

		VADONCOMMON_API std::string get_entity_name(EntityHandle entity_handle) const;
		VADONCOMMON_API void set_entity_name(EntityHandle entity_handle, std::string_view name);

		VADONCOMMON_API EntityHandle get_entity_parent(EntityHandle entity) const;
		VADONCOMMON_API EntityHandle get_entity_root(EntityHandle entity) const;
		VADONCOMMON_API void set_entity_parent(EntityHandle entity, EntityHandle parent);
		void add_child_entity(EntityHandle parent, EntityHandle child) { set_entity_parent(child, parent); }
		VADONCOMMON_API void remove_child_entity(EntityHandle parent, EntityHandle child);

		VADONCOMMON_API bool is_ancestor(EntityHandle entity, EntityHandle ancestor) const;
		VADONCOMMON_API EntityList get_children(EntityHandle entity, bool recursive = false) const;

		VADONCOMMON_API EntityList get_active_entities() const;
	private:
		struct EntityData
		{
			std::string name;
			EntityHandle parent;
			EntityList children;
		};

		EntityManager();

		VADONCOMMON_API EntityList remove_entity(EntityHandle entity_handle);

		void internal_add_child_entity(EntityHandle parent, EntityHandle child);
		void internal_remove_child_entity(EntityHandle parent, EntityHandle child);

		void clear();

		using EntityPool = Vadon::Utilities::ObjectPool<Vadon::ECS::Entity, EntityData>;
		EntityPool m_entity_pool;

		friend class World;
	};
}
#endif