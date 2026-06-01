#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

namespace VadonEditor::Model
{
	SceneSystem::~SceneSystem()
	{
		Q_ASSERT_X(m_scene_lookup.empty() == true, "VadonEditor::Model::SceneSystem::SceneSystem", "System was not shut down correctly");
	}

	Scene* SceneSystem::create_scene()
	{
		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		Model::Resource* scene_resource = resource_system.create_resource(Scene::get_scene_type_uuid());
		
		if (scene_resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::create_scene", "Failed to create scene resource");
			return nullptr;
		}

		// NOTE: save the contents of the empty scene into the Resource
		Model::Scene* new_scene = internal_add_new_scene(scene_resource);
		if (new_scene->store_scene_data() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::create_scene", "Failed to store scene data");
			return nullptr;
		}

		return new_scene;
	}

	Scene* SceneSystem::get_scene(const SceneID& scene_id)
	{
		auto scene_it = m_scene_lookup.find(scene_id);
		if (scene_it != m_scene_lookup.end())
		{
			// Scene already loaded
			return scene_it.value();
		}

		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		Resource* scene_resource = resource_system.get_resource(scene_id);

		if (scene_resource == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::get_scene", "Failed to get scene resource");
			return nullptr;
		}

		Model::Scene* new_scene = internal_add_new_scene(scene_resource);
		if (new_scene->load_scene() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::get_scene", "Failed to load scene data");
			return nullptr;
		}

		return new_scene;
	}

	void SceneSystem::remove_scene(Scene* scene)
	{
		if (scene->m_resource != nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::remove_scene", "Cannot remove active scene!");
			return;
		}

		auto scene_it = m_scene_lookup.find(scene->m_id);
		if (scene_it == m_scene_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::ResourceSystem::remove_scene", "Scene was not in lookup!");
			return;
		}

		m_scene_lookup.erase(scene_it);
	}

	int SceneSystem::create_scene_asset(const SceneID& scene_id, const QString& path)
	{
		return m_application.get_model_system().get_resource_system().create_resource_asset(scene_id, path);
	}

	int SceneSystem::find_scene_asset(const SceneID& scene_id) const
	{
		return m_application.get_model_system().get_resource_system().find_resource_asset_id(scene_id);
	}

	SceneSystem::SceneSystem(Core::Application& application)
		: m_application(application)
	{
	}

	bool SceneSystem::initialize()
	{
		return true;
	}

	void SceneSystem::project_loaded()
	{
		// TODO: anything?
	}

	void SceneSystem::shutdown()
	{
		// Clean up any leftover resources
		for (auto scene_it = m_scene_lookup.begin(); scene_it != m_scene_lookup.end(); ++scene_it)
		{
			Scene* current_scene = scene_it.value();

			// Clear resource pointer to skip internal management, everything will be removed
			current_scene->m_resource = nullptr;

			delete current_scene;
		}

		m_scene_lookup.clear();
	}

	Scene* SceneSystem::internal_add_new_scene(Resource* scene_resource)
	{
		if (scene_resource->get_info().type != Scene::get_scene_type_uuid())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::internal_add_new_scene", "Resource is not the correct type");
			return nullptr;
		}

		auto scene_it = m_scene_lookup.find(scene_resource->get_info().id);
		if (scene_it != m_scene_lookup.end())
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::internal_add_new_scene", "Scene already added with this ID");
			return nullptr;
		}

		Scene* new_scene = new Scene(m_application, scene_resource);
		if (new_scene->initialize() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::Model::SceneSystem::internal_add_new_scene", "Failed to initialize scene");
			delete new_scene;
			return nullptr;
		}

		m_scene_lookup.insert(scene_resource->get_info().id, new_scene);
		return new_scene;
	}
}