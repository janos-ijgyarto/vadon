#include <VadonEditor/UI/Model/Scene/SceneManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/Network/Message/MessageSerializer.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>
#include <Vadon/Foundation/Model/Scene/Scene.hpp>

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
		SceneTree* scene_tree = find_scene_tab(resource_info.id);
		if (scene_tree != nullptr)
		{
			scene_tab_widget->setCurrentWidget(scene_tree);
			return;
		}

		// Scene does not have tab yet, create one
		SceneTree* new_scene_tree = new SceneTree(opened_scene);
		connect(new_scene_tree, &SceneTree::scene_modified, this, &SceneManager::scene_modified);
		connect(new_scene_tree, &SceneTree::scene_saved, this, &SceneManager::scene_saved);

		const int new_tab_index = scene_tab_widget->addTab(new_scene_tree, asset_info.path);

		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		scene_tab_bar->setTabData(new_tab_index, resource_info.id);
	}

	void SceneManager::scene_modified(const QUuid& scene_id)
	{
		SceneTree* scene_tree = find_scene_tab(scene_id);
		if (scene_tree == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::scene_modified", "Cannot find scene");
			return;
		}

		update_scene_tab_label(scene_tree);
	}

	void SceneManager::scene_saved(const QUuid& scene_id)
	{
		SceneTree* scene_tree = find_scene_tab(scene_id);
		if (scene_tree == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::scene_saved", "Cannot find scene");
			return;
		}

		update_scene_tab_label(scene_tree);
	}

	void SceneManager::current_scene_changed(int tab_index)
	{
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();

		QWidget* selected_tab = scene_tab_widget->widget(tab_index);
		SceneTree* selected_scene_tree = qobject_cast<SceneTree*>(selected_tab);

		{
			// FIXME: use temp allocator or shared serializer
			VadonEditor::Network::MessageSerializer message_serializer;

			::Vadon::Foundation::EditorModelMessageSceneSelected scene_selected;
			scene_selected.message_type = ::Vadon::Foundation::EditorModelMessageType::SCENE_SELECTED;

			scene_selected.scene_id = Utilities::qt_uuid_to_vadon_uuid(selected_scene_tree->get_scene()->get_id());

			message_serializer.write_message_trivial(::Vadon::Foundation::EditorMessageCategory::MODEL, scene_selected);
		}
	}

	SceneManager::SceneManager(Core::Application& application)
		: m_application(application)
	{

	}

	bool SceneManager::initialize()
	{
		connect(&m_application.get_asset_manager(), &Core::AssetManager::asset_opened, this, &SceneManager::asset_opened);
		
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		connect(scene_tab_widget, &QTabWidget::currentChanged, this, &SceneManager::current_scene_changed);

		return true;
	}

	void SceneManager::shutdown()
	{
		// TODO: check that all widgets have been closed!
	}

	bool SceneManager::request_close()
	{
		// Check all scene trees to make sure we have no unsaved changes
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		for (int tab_index = 0; tab_index < scene_tab_bar->count(); ++tab_index)
		{
			QWidget* current_tab_widget = scene_tab_widget->widget(tab_index);
			SceneTree* current_scene_tree = qobject_cast<SceneTree*>(current_tab_widget);
			if (current_scene_tree == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::request_close", "Invalid widget in tabs");
				continue;
			}

			if (current_scene_tree->request_close() == false)
			{
				return false;
			}
		}

		return true;
	}

	void SceneManager::force_close()
	{
		// Force close all widgets attached to scenes
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		for (int tab_index = 0; tab_index < scene_tab_bar->count(); ++tab_index)
		{
			QWidget* current_tab_widget = scene_tab_widget->widget(tab_index);
			SceneTree* current_scene_tree = qobject_cast<SceneTree*>(current_tab_widget);
			if (current_scene_tree == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneManager::force_close", "Invalid widget in tabs");
				continue;
			}

			current_scene_tree->force_close();
		}
	}

	SceneTree* SceneManager::find_scene_tab(const QUuid& scene_id) const
	{
		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		QTabBar* scene_tab_bar = scene_tab_widget->tabBar();
		for (int tab_index = 0; tab_index < scene_tab_bar->count(); ++tab_index)
		{
			const QUuid tab_scene_uuid = scene_tab_bar->tabData(tab_index).toUuid();
			if (tab_scene_uuid == scene_id)
			{
				SceneTree* scene_tree = qobject_cast<SceneTree*>(scene_tab_widget->widget(tab_index));
				Q_ASSERT_X(scene_tree != nullptr, "VadonEditor::UI::SceneManager::find_scene_tab", "Invalid widget in scene tabs");
				return scene_tree;
			}
		}

		return nullptr;
	}

	void SceneManager::update_scene_tab_label(SceneTree* scene_tab) const
	{
		QString scene_tab_label = scene_tab->get_label();
		if (scene_tab->get_scene()->is_modified() == true)
		{
			scene_tab_label += " (*)";
		}

		QTabWidget* scene_tab_widget = m_application.get_ui_system().get_main_window()->get_scene_tab_widget();
		const int tab_index = scene_tab_widget->indexOf(scene_tab);

		scene_tab_widget->setTabText(tab_index, scene_tab_label);
	}
}