#include <VadonEditor/UI/Model/Scene/EntityEditor.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>
#include <VadonEditor/Model/Scene/SceneSystem.hpp>

#include <VadonEditor/UI/Model/Scene/AddComponentDialog.hpp>
#include <VadonEditor/UI/Model/Scene/ComponentWidget.hpp>

namespace VadonEditor::UI
{
	EntityEditor::EntityEditor(Model::Entity* entity, QWidget* parent)
		: QWidget(parent)
		, m_entity(entity)
	{
		m_ui.setupUi(this);
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);
	}

	void EntityEditor::name_changed(const QString& text)
	{
		m_entity->set_name(text);
		update_title();
	}

	void EntityEditor::add_component_clicked()
	{
		AddComponentDialog* add_component_dialog = new AddComponentDialog(m_entity->get_owner_scene().get_application(), m_entity, this);
		connect(add_component_dialog, &AddComponentDialog::component_type_selected, this, &EntityEditor::new_component_selected);

		add_component_dialog->open();
	}

	void EntityEditor::new_component_selected(const QUuid& component_type)
	{
		qDebug() << "Requested component type" << component_type.toString();
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
			Model::Scene& entity_scene = m_entity->get_owner_scene();

			Core::Application& application = entity_scene.get_application();
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

		for (auto component_it = m_entity->get_components().begin(); component_it != m_entity->get_components().end(); ++component_it)
		{
			ComponentWidget* component_widget = new ComponentWidget(m_entity, &component_it.value());
			if (component_widget->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::EntityEditor::initialize", "Failed to initialize component widget!");
				return false;
			}

			const int spacer_index = m_ui.componentListVBox->indexOf(m_ui.componentListSpacer);
			m_ui.componentListVBox->insertWidget(spacer_index, component_widget);
		}

		return true;
	}

	bool EntityEditor::update_title()
	{
		if (parentWidget() != nullptr)
		{
			// Widget is embedded in parent, so we don't set a title
			return true;
		}

		Model::Scene& entity_scene = m_entity->get_owner_scene();
		Core::Application& application = entity_scene.get_application();

		Model::SceneSystem& scene_system = application.get_model_system().get_scene_system();

		// No parent, opened as separate window, so we should set a title
		const int scene_asset_id = scene_system.find_scene_asset(entity_scene.get_resource()->get_info().id);

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
}