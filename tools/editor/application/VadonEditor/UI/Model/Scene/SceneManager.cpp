#include <VadonEditor/UI/Model/Scene/SceneManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>
#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

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

		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		const Model::ResourceInfo resource_info = resource_system.resource_info_by_asset_id(asset_info.id);

		Q_ASSERT_X(resource_info.is_valid() == true, "VadonEditor::UI::SceneManager::asset_opened", "Cannot find scene resource");

		Model::SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
		Model::Scene* opened_scene = scene_system.get_scene(resource_info.id);

		Q_ASSERT_X(opened_scene != nullptr, "VadonEditor::UI::SceneManager::asset_opened", "Failed to get scene");

		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		for (int tab_index = 0; tab_index < scene_tab_bar->count(); ++tab_index)
		{
			const QUuid tab_scene_uuid = scene_tab_bar->tabData(tab_index).toUuid();
			if (tab_scene_uuid == resource_info.id)
			{
				scene_tab_widget->setCurrentIndex(tab_index);
				return;
			}
		}

		// Scene does not have tab yet, create one
		SceneTree* new_scene_tree = new SceneTree(opened_scene);
		connect(new_scene_tree, &SceneTree::entity_opened, this, &SceneManager::entity_opened);

		const int new_tab_index = scene_tab_widget->addTab(new_scene_tree, asset_info.path);
		scene_tab_bar->setTabData(new_tab_index, resource_info.id);
	}

	void SceneManager::entity_opened(const QUuid& scene_id, const QModelIndex& index)
	{
		Model::SceneSystem& scene_system = m_application.get_model_system().get_scene_system();
		Model::Scene* entity_scene = scene_system.get_scene(scene_id);

		Model::EntityModel& entity_model = entity_scene->get_entity_model();
		Model::Entity* opened_entity = entity_model.get_entity_by_model_index(index);
		Q_ASSERT_X(opened_entity != nullptr, "VadonEditor::UI::SceneManager::entity_opened", "Cannot find entity");

		auto entity_widget_it = m_entity_widgets.find(opened_entity);
		if (entity_widget_it != m_entity_widgets.end())
		{
			QWidget* editor_widget = entity_widget_it.value();
			editor_widget->setWindowState((editor_widget->windowState() & ~Qt::WindowState::WindowMinimized) | Qt::WindowState::WindowActive);
			editor_widget->raise();
			editor_widget->activateWindow();
		}
		else
		{
			EntityEditor* new_entity_editor = new EntityEditor(opened_entity);
			if (new_entity_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::entity_opened", "Failed to initialize Entity editor!");
				delete new_entity_editor;
				return;
			}

			m_entity_widgets.insert(opened_entity, new_entity_editor);
			m_widget_reverse_lookup.insert(new_entity_editor, opened_entity);

			connect(new_entity_editor, &QObject::destroyed, this, &SceneManager::entity_widget_removed);

			new_entity_editor->show();
		}
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

	void SceneManager::shutdown()
	{
		// TODO: check that all widgets have been closed!
	}

	bool SceneManager::close_requested()
	{
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		for (int tab_index = 0; tab_index < scene_tab_bar->count(); ++tab_index)
		{
			QWidget* current_tab_widget = scene_tab_widget->widget(tab_index);
			SceneTree* current_scene_tree = qobject_cast<SceneTree*>(current_tab_widget);
			if (current_scene_tree == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::close_requested", "Invalid widget in tabs");
				continue;
			}

			if (current_scene_tree->close_requested() == false)
			{
				return false;
			}
		}

		for (auto entity_widget_it = m_entity_widgets.begin(); entity_widget_it != m_entity_widgets.end(); ++entity_widget_it)
		{
			EntityEditor* entity_widget = qobject_cast<EntityEditor*>(m_entity_widgets.begin().value());
			if (entity_widget == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::close_requested", "Invalid widget in lookup");
				continue;
			}

			if (entity_widget->request_close() == false)
			{
				return false;
			}
		}

		return true;
	}

	void SceneManager::force_close()
	{
		// Clear reverse lookup so we can ignore the signal from when the widgets are destroyed on close
		m_widget_reverse_lookup.clear();

		for (auto entity_widget_it = m_entity_widgets.begin(); entity_widget_it != m_entity_widgets.end(); ++entity_widget_it)
		{
			entity_widget_it.value()->close();
		}

		m_entity_widgets.clear();
	}
}