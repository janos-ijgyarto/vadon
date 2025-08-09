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
		bool is_entity_pending_remove(EntityHandle entity_handle) const;
		VADONCOMMON_API void remove_entity(EntityHandle entity_handle);

		VADONCOMMON_API std::string get_entity_name(EntityHandle entity_handle) const;
		VADONCOMMON_API void set_entity_name(EntityHandle entity_handle, std::string_view name);

		VADONCOMMON_API EntityHandle get_entity_parent(EntityHandle entity) const;
		VADONCOMMON_API EntityHandle get_entity_root(EntityHandle entity) const;
		VADONCOMMON_API void set_entity_parent(EntityHandle entity, EntityHandle parent);
		void add_child_entity(EntityHandle parent, EntityHandle child) { set_entity_parent(child, parent); }
		VADONCOMMON_API void remove_child_entity(EntityHandle parent, EntityHandle child); // NOTE: child is only removed from parent, not removed altogether!

		VADONCOMMON_API bool is_ancestor(EntityHandle entity, EntityHandle ancestor) const;
		VADONCOMMON_API EntityList get_children(EntityHandle entity, bool recursive = false) const;

		VADONCOMMON_API EntityList get_active_entities() const;
	private:
		struct EntityData
		{
			std::string name;
			EntityHandle parent;
			EntityList children;
			bool pending_remove = false;

			void remove_child(EntityHandle child);
		};

		EntityManager();

		void internal_add_child_entity(EntityHandle parent, EntityHandle child);

		void internal_set_entity_pending_remove(EntityHandle entity, EntityData& entity_data);

		void remove_pending_entities();
		void clear();

		using EntityPool = Vadon::Utilities::ObjectPool<Vadon::ECS::Entity, EntityData>;
		EntityPool m_entity_pool;
		std::vector<EntityHandle> m_pending_remove_list;

		friend class World;
	};
}
#endif