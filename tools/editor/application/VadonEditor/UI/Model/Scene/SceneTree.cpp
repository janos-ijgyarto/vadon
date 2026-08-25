#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceDialog.hpp>

#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	SceneTree::SceneTree(Model::Scene* scene, QWidget* parent)
		: QWidget(parent)
		, m_scene(scene)
	{
		m_ui.setupUi(this);

		m_ui.treeView->addAction(m_ui.actionSave);
		m_ui.treeView->setModel(&scene->get_entity_model().get_qt_model());
	}

	QString SceneTree::get_label() const
	{
		Core::Application& application = m_scene->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
		const int scene_asset_id = resource_system.find_resource_asset_id(m_scene->get_id());

		Core::AssetManager& asset_manager = application.get_asset_manager();
		const QModelIndex asset_index = asset_manager.find_asset_index(scene_asset_id);

		const Core::AssetInfo asset_info = asset_manager.get_asset_info(asset_index);

		return asset_info.path;
	}

	void SceneTree::entity_double_clicked(const QModelIndex& index)
	{
		Model::EntityModel& entity_model = m_scene->get_entity_model();
		Model::Entity* opened_entity = entity_model.get_entity_by_model_index(index);
		if (opened_entity == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::SceneTree::entity_double_clicked", "Cannot find entity");
			return;
		}

		entity_opened(opened_entity);
	}

	void SceneTree::entity_widget_removed(QObject* widget_obj)
	{
		QWidget* entity_widget = qobject_cast<QWidget*>(widget_obj);
		auto entity_it = m_widget_reverse_lookup.find(entity_widget);
		if (entity_it != m_widget_reverse_lookup.end())
		{
			m_entity_widgets.remove(entity_it.value());
			m_widget_reverse_lookup.erase(entity_it);
		}
	}

	void SceneTree::save_triggered()
	{
		if (m_scene->save_scene() == true)
		{
			emit(scene_saved(m_scene->get_resource()->get_info().id));
		}
	}

	void SceneTree::add_entity_triggered()
	{
		Model::Entity* selected_entity = get_selected_entity();
		if (selected_entity == nullptr)
		{
			qWarning() << "Entity add was requested, but no Entity was selected!";
			return;
		}

		m_scene->get_entity_model().add_entity(selected_entity);
		set_modified();
	}

	void SceneTree::instantiate_scene_triggered()
	{
		Model::Entity* selected_entity = get_selected_entity();
		if (selected_entity == nullptr)
		{
			qWarning() << "Instantiate scene was requested, but no Entity was selected!";
			return;
		}

		SelectResourceDialog* select_dialog = new SelectResourceDialog(m_scene->get_application(), Model::Scene::get_scene_type_uuid(), this);
		connect(select_dialog, &SelectResourceDialog::resource_asset_selected, this, &SceneTree::scene_asset_selected);

		select_dialog->open();
	}

	void SceneTree::remove_entity_triggered()
	{
		Model::Entity* selected_entity = get_selected_entity();
		if (selected_entity == nullptr)
		{
			qWarning() << "Entity remove was requested, but no Entity was selected!";
			return;
		}

		m_scene->get_entity_model().remove_entity(selected_entity->get_id());
		set_modified();
	}

	void SceneTree::entity_context_menu_requested(const QPoint& position)
	{
#ifndef QT_NO_CONTEXTMENU
		const QModelIndex entity_index = m_ui.treeView->indexAt(position);
		if (entity_index.isValid() == false)
		{
			return;
		}

		QMenu menu(this);

		menu.addAction(m_ui.actionAddEntity);
		menu.addAction(m_ui.actionInstantiateScene);
		menu.addAction(m_ui.actionRemoveEntity);

		menu.exec(m_ui.treeView->mapToGlobal(position));
#else
		Q_UNUSED(position);
#endif
	}

	void SceneTree::internal_scene_modified()
	{
		set_modified();
	}

	void SceneTree::scene_asset_selected(const QUuid& scene_id)
	{
		Model::Entity* selected_entity = get_selected_entity();
		if (selected_entity == nullptr)
		{
			qWarning() << "Scene asset was selected, but no parent Entity was selected!";
			return;
		}

		m_scene->instantiate_scene(scene_id, selected_entity);
		set_modified();
	}

	void SceneTree::entity_opened(Model::Entity* entity)
	{
		auto entity_widget_it = m_entity_widgets.find(entity->get_id());
		if (entity_widget_it != m_entity_widgets.end())
		{
			QWidget* editor_widget = entity_widget_it.value();
			editor_widget->setWindowState((editor_widget->windowState() & ~Qt::WindowState::WindowMinimized) | Qt::WindowState::WindowActive);
			editor_widget->raise();
			editor_widget->activateWindow();
		}
		else
		{
			EntityEditor* new_entity_editor = new EntityEditor(m_scene, entity, m_scene->get_application().get_ui_system().get_main_window(), Qt::WindowType::Window);
			if (new_entity_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::SceneTree::entity_opened", "Failed to initialize Entity editor!");
				delete new_entity_editor;
				return;
			}

			m_entity_widgets.insert(entity->get_id(), new_entity_editor);
			m_widget_reverse_lookup.insert(new_entity_editor, entity->get_id());

			connect(new_entity_editor, &QObject::destroyed, this, &SceneTree::entity_widget_removed);

			connect(entity, &Model::Entity::name_changed, this, &SceneTree::internal_scene_modified);
			connect(entity, &Model::Entity::component_added, this, &SceneTree::internal_scene_modified);
			connect(entity, &Model::Entity::component_removed, this, &SceneTree::internal_scene_modified);
			connect(entity, &Model::Entity::component_property_edited, this, &SceneTree::internal_scene_modified);

			new_entity_editor->show();
		}
	}

	void SceneTree::internal_close()
	{
		// Clear reverse lookup so we can ignore the signal from when the widgets are destroyed on close
		m_widget_reverse_lookup.clear();

		for (auto entity_widget_it = m_entity_widgets.begin(); entity_widget_it != m_entity_widgets.end(); ++entity_widget_it)
		{
			entity_widget_it.value()->close();
		}

		m_entity_widgets.clear();
	}

	void SceneTree::set_modified()
	{ 
		m_scene->notify_modified();
		emit(scene_modified(m_scene->get_resource()->get_info().id));
	}

	Model::Entity* SceneTree::get_selected_entity() const
	{
		QModelIndexList selected_indexes = m_ui.treeView->selectionModel()->selectedIndexes();
		if (selected_indexes.isEmpty() == false)
		{
			return m_scene->get_entity_model().get_entity_by_model_index(selected_indexes.front());
		}

		return nullptr;
	}
}