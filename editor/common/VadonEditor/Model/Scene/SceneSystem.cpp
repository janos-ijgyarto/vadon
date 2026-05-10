#include <VadonEditor/Model/Scene/SceneSystem.hpp>

namespace VadonEditor::Model
{
	Scene* SceneSystem::get_scene(Vadon::Model::SceneID /*scene_id*/)
	{
		// TODO!!
		return nullptr;
	}

	void SceneSystem::add_entity_event_callback(EntityEventCallback callback)
	{
		m_entity_callbacks.push_back(callback);
	}

	void SceneSystem::add_component_event_callback(ComponentEventCallback callback)
	{
		m_component_callbacks.push_back(callback);
	}

	const Scene* SceneSystem::find_entity_scene(Vadon::ECS::EntityHandle /*entity*/) const
	{
		// TODO!!!
		return nullptr;
	}
}