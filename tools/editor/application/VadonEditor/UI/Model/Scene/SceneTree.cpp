#include <VadonEditor/UI/Model/Scene/SceneTree.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>

#include <QMessageBox>

namespace VadonEditor::UI
{
	SceneTree::SceneTree(Model::Scene* scene, QWidget* parent)
		: QWidget(parent)
		, m_scene(scene)
	{
		m_ui.setupUi(this);

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

	void SceneTree::entity_name_changed(const QUuid& entity_id, const QString& text)
	{
		Model::EntityModel& entity_model = m_scene->get_entity_model();
		Model::Entity* entity = entity_model.find_entity_by_id(entity_id);

		entity_model.set_entity_name(entity, text);

		set_modified();
	}

	void SceneTree::entity_component_added(const QUuid& entity_id, const QUuid& component_id)
	{
		Q_UNUSED(entity_id);
		Q_UNUSED(component_id);
		set_modified();
	}

	void SceneTree::entity_component_removed(const QUuid& entity_id, const QUuid& component_id)
	{
		Q_UNUSED(entity_id);
		Q_UNUSED(component_id);
		set_modified();
	}

	void SceneTree::entity_component_data_changed(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id)
	{
		// TODO: handle the property changes!
		Q_UNUSED(entity_id);
		Q_UNUSED(component_id);
		Q_UNUSED(property_id);
		set_modified();
	}

	void SceneTree::save_triggered()
	{
		// TODO: save scene data!
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
			connect(new_entity_editor, &EntityEditor::entity_name_changed, this, &SceneTree::entity_name_changed);
			connect(new_entity_editor, &EntityEditor::component_added, this, &SceneTree::entity_component_added);
			connect(new_entity_editor, &EntityEditor::component_removed, this, &SceneTree::entity_component_removed);
			connect(new_entity_editor, &EntityEditor::component_property_edited, this, &SceneTree::entity_component_data_changed);

			new_entity_editor->show();
		}
	}

	bool SceneTree::request_close()
	{
		if (m_scene->is_modified())
		{
			QMessageBox message_box(this);
			message_box.setWindowTitle("Unsaved changes in Scene");
			message_box.setText(QString("Save changes to scene \"%1?\"").arg(get_label())); 
			message_box.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			message_box.setDefaultButton(QMessageBox::Yes);
			message_box.setIcon(QMessageBox::Icon::Question);

			const int user_response = message_box.exec();
			switch (user_response)
			{
			case QMessageBox::StandardButton::Yes:
				// TODO: save changes
				break;
			case QMessageBox::StandardButton::Cancel:
				return false;
			}
		}

		return true;
	}

	void SceneTree::force_close()
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
}