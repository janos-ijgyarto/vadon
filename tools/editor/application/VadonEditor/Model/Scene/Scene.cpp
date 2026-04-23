#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace VadonEditor::Model
{
	Scene::~Scene()
	{
		if (m_resource == nullptr)
		{
			// Special case: Scene System is cleaning up, so we can early out
			return;
		}

		Resource* resource = m_resource;
		m_resource = nullptr;

		// Unregister from Scene system
		m_application.get_model_system().get_scene_system().remove_scene(this);

		// Delete the Resource
		// TODO: decrement refcount instead!
		delete resource;
	}

	bool Scene::is_scene_base_of_type(Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(get_scene_type_uuid(), type_id);
	}

	QUuid Scene::get_scene_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid);
	}

	QUuid Scene::get_scene_entities_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_entities_property.id);
	}

	QUuid Scene::get_scene_resource_data_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_scene_resource_data_uuid);
	}

	bool Scene::initialize()
	{
		// TODO: anything?
		return true;
	}

	bool Scene::store_scene_data() const
	{
		QVariantMap entity_data_map;
		QVariantList entity_list;

		if (m_entity_model.save_model(m_resource->get_application(), entity_list) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Scene::store_scene_data", "Failed to save scene data");
			return false;
		}
		
		entity_data_map.insert(Utilities::serialize_labeled_uuid("entities", get_scene_entities_uuid()), entity_list);

		m_resource->set_data(get_scene_resource_data_uuid(), entity_data_map);
		return true;
	}

	bool Scene::save_scene() const
	{
		if (store_scene_data() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::Scene::save_scene", "Failed to save scene data");
			return false;
		}

		return m_application.get_model_system().get_resource_system().save_resource(m_resource);
	}

	bool Scene::load_scene()
	{
		const QVariantMap entity_data_map = m_resource->get_data(get_scene_resource_data_uuid()).toMap();

		bool entity_data_found = false;
		for (auto entity_data_it = entity_data_map.begin(); entity_data_it != entity_data_map.end(); ++entity_data_it)
		{
			const QUuid entry_uuid = Utilities::parse_labeled_uuid(entity_data_it.key());
			if (entry_uuid == get_scene_entities_uuid())
			{
				const QVariantList entity_list = entity_data_it.value().toList();
				if (m_entity_model.load_model(m_resource->get_application(), entity_list) == false)
				{
					Q_ASSERT_X(false, "VadonEditor::Model::Scene::load_scene", "Failed to load scene entity data");
					return false;
				}
				entity_data_found = true;
			}
			else
			{
				// TODO: warn about unrecognized data?
			}
		}

		return entity_data_found;
	}

	Scene::Scene(Core::Application& application, Resource* resource)
		: m_application(application)
		, m_id(resource->get_info().id)
		, m_resource(resource)
		, m_entity_model(*this)
	{

	}
}