#include <VadonEditor/UI/Model/Scene/SceneManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>

namespace VadonEditor::UI
{
	void SceneManager::asset_opened(const QModelIndex& index)
	{
		const Core::AssetInfo asset_info = m_application.get_asset_manager().get_asset_info(index);
		if (asset_info.type != Core::AssetType::SCENE)
		{
			// Make sure we only try to open Scene assets
			return;
		}

		Model::SceneSystem& scene_system = m_application.get_model_system().get_scene_system();

		// TODO: add tab to scene tabs
		Q_UNUSED(scene_system);
	}

	void SceneManager::entity_widget_removed(QObject* widget_obj)
	{
		QWidget* entity_widget = qobject_cast<QWidget*>(widget_obj);
		auto entity_it = m_widget_reverse_lookup.find(entity_widget);
		if (entity_it != m_widget_reverse_lookup.end())
		{
			m_entity_widgets.remove(entity_it.value());
			m_widget_reverse_lookup.erase(entity_it);
		}
	}

	SceneManager::SceneManager(Core::Application& application)
		: m_application(application)
	{

	}

	bool SceneManager::initialize()
	{
		connect(&m_application.get_asset_manager(), &Core::AssetManager::asset_opened, this, &SceneManager::asset_opened);
		return true;
	}

	void SceneManager::entity_opened()
	{
		// TODO: open window to edit Entity!
	}
}