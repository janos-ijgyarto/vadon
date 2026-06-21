#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/Model/Scene/AddComponentDialog.hpp>
#include <VadonEditor/UI/Model/Scene/ComponentWidget.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::UI
{
	void EntityEditor::closeEvent(QCloseEvent* event)
	{
		store_entity_data();

		QWidget::closeEvent(event);
	}

	void EntityEditor::internal_name_changed(const QString& text)
	{
		m_entity->set_name(text);
		update_title();
	}

	void EntityEditor::add_component_clicked()
	{
		const QList<QUuid> component_id_list = m_entity->get_component_id_list();

		AddComponentDialog* add_component_dialog = new AddComponentDialog(m_entity->get_application(), component_id_list, this);
		connect(add_component_dialog, &AddComponentDialog::component_type_selected, this, &EntityEditor::new_component_selected);

		add_component_dialog->open();
	}

	void EntityEditor::new_component_selected(const QUuid& component_type)
	{
		{
			Model::Component* existing_component = m_entity->get_component(component_type);
			if (existing_component != nullptr)
			{
				qWarning() << "Component type" << Utilities::uuid_to_base64_string(component_type) << "already added!";
				return;
			}
		}

		Model::Component* added_component = m_entity->add_component(component_type);
		if (added_component == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::new_component_selected", "Failed to add component!");
			return;
		}

		if (internal_add_component_widget(added_component) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::new_component_selected", "Failed to add component widget");
		}
	}

	void EntityEditor::component_remove_requested(const QUuid& component_id)
	{
		Model::Component* existing_component = m_entity->get_component(component_id);
		if (existing_component != nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::component_remove_requested", "Component not found in Entity");
			return;
		}

		for (int item_index = 0; item_index < m_ui.componentListVBox->count(); ++item_index)
		{
			QWidget* current_widget = m_ui.componentListVBox->itemAt(item_index)->widget();
			ComponentWidget* component_widget = qobject_cast<ComponentWidget*>(current_widget);
			if (component_widget != nullptr)
			{
				if (component_widget->get_component()->get_type_id() == component_id)
				{
					QLayoutItem* layout_item = m_ui.componentListVBox->takeAt(item_index);
					delete layout_item->widget();
					delete layout_item;

					m_entity->remove_component(component_id);
					return;
				}
			}
		}
	}

	EntityEditor::EntityEditor(Model::Scene* scene, Model::Entity* entity, QWidget* parent, Qt::WindowType type)
		: QWidget(parent, type)
		, m_scene(scene)
		, m_entity(entity)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
	}

	bool EntityEditor::initialize()
	{
		if (m_entity == nullptr)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Entity must not be null!");
			return false;
		}

		if (update_title() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Failed to set title!");
			return false;
		}

		m_ui.nameLineEdit->setText(m_entity->get_name());

		if (m_entity->get_sub_scene_id().isNull() == false)
		{
			Core::Application& application = m_entity->get_application();
			Model::SceneSystem& scene_system = application.get_model_system().get_scene_system();

			// No parent, opened as separate window, so we should set a title
			const int sub_scene_asset_id = scene_system.find_scene_asset(m_entity->get_sub_scene_id());

			Core::AssetManager& asset_manager = application.get_asset_manager();
			const QModelIndex asset_index = asset_manager.find_asset_index(sub_scene_asset_id);
			if (asset_index.isValid() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Cannot find sub-scene asset");
				return false;
			}

			const Core::AssetInfo& sub_scene_asset_info = asset_manager.get_asset_info(asset_index);
			m_ui.subSceneResourceLabel->setText(sub_scene_asset_info.path);
		}
		else
		{
			m_ui.subSceneLabel->setVisible(false);
			m_ui.subSceneResourceLabel->setVisible(false);
		}

		const QList<QUuid> component_id_list = m_entity->get_component_id_list();

		// TODO: sort component widgets by name?
		for (const QUuid& current_component_id : component_id_list)
		{
			Model::Component* current_component = m_entity->get_component(current_component_id);
			if (internal_add_component_widget(current_component) == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Failed to add component widget");
				return false;
			}
		}

		return true;
	}

	bool EntityEditor::update_title()
	{
		if (windowType() != Qt::WindowType::Window)
		{
			// Widget is embedded in parent, so we don't set a title
			return true;
		}

		Core::Application& application = m_entity->get_application();
		Model::SceneSystem& scene_system = application.get_model_system().get_scene_system();

		// No parent, opened as separate window, so we should set a title
		const int scene_asset_id = scene_system.find_scene_asset(m_scene->get_id());

		Core::AssetManager& asset_manager = application.get_asset_manager();
		const QModelIndex asset_index = asset_manager.find_asset_index(scene_asset_id);
		if (asset_index.isValid() == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Cannot find asset");
			return false;
		}

		const Core::AssetInfo& scene_asset_info = asset_manager.get_asset_info(asset_index);
		setWindowTitle(QString("Entity Editor - %1 (%2)").arg(m_entity->get_label()).arg(scene_asset_info.path));

		return true;
	}

	bool EntityEditor::internal_add_component_widget(Model::Component* component)
	{
		ComponentWidget* component_widget = new ComponentWidget(component);
		if (component_widget->initialize(m_scene) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::internal_add_component_widget", "Failed to initialize component widget!");
			delete component_widget;
			return false;
		}

		connect(component_widget, &ComponentWidget::remove_requested, this, &EntityEditor::component_remove_requested);

		const int spacer_index = m_ui.componentListVBox->indexOf(m_ui.componentListSpacer);
		m_ui.componentListVBox->insertWidget(spacer_index, component_widget);

		return true;
	}

	void EntityEditor::store_entity_data()
	{
		m_entity->store_component_data();
	}
}