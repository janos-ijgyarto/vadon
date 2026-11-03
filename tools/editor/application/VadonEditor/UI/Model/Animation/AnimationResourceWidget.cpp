#include <VadonEditor/UI/Model/Animation/AnimationResourceWidget.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/Model/Animation/AnimationEditor.hpp>

#include <QToolBar>

namespace VadonEditor::UI
{
	AnimationResourceWidget::AnimationResourceWidget(Model::Resource* resource, QWidget* parent)
		: QWidget(parent)
		, m_resource(resource)
		, m_animation_model(nullptr)
		, m_animation_editor(nullptr)
	{
		setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose, true);

		m_ui.setupUi(this);

		// Create model object
		m_animation_model = new Model::Animation(resource->get_application());
		if (m_animation_model->load_from_resource(resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::AnimationResourceWidget::AnimationResourceWidget", "Failed to load animation data");
			m_animation_model->deleteLater();
			return;
		}

		connect(m_animation_model, &Model::Animation::channel_added, this, &AnimationResourceWidget::channel_added);
		connect(m_animation_model, &Model::Animation::channel_edited, this, &AnimationResourceWidget::channel_edited);
		connect(m_animation_model, &Model::Animation::channel_removed, this, &AnimationResourceWidget::channel_removed);

		m_animation_editor = new AnimationEditor(*m_animation_model, this);
		m_ui.mainLayout->addWidget(m_animation_editor, 1);

		// No parent, can add toolbar
		QToolBar* toolbar = new QToolBar();
		toolbar->addAction(m_ui.actionSave);

		m_ui.mainLayout->setMenuBar(toolbar);
	}

	AnimationResourceWidget::~AnimationResourceWidget()
	{
		m_animation_model->deleteLater();
	}

	void AnimationResourceWidget::channel_added(const QUuid& animation_id, const QUuid& channel_id)
	{
		// FIXME: isolate update to what changed, instead of re-exporting the whole resource
		Q_UNUSED(animation_id);
		Q_UNUSED(channel_id);
		m_animation_model->store_to_resource(m_resource);

		update_title();
	}

	void AnimationResourceWidget::channel_edited(const QUuid& animation_id, const QUuid& channel_id)
	{
		// FIXME: isolate update to what changed, instead of re-exporting the whole resource
		Q_UNUSED(animation_id);
		Q_UNUSED(channel_id);
		m_animation_model->store_to_resource(m_resource);

		update_title();
	}

	void AnimationResourceWidget::channel_removed(const QUuid& animation_id, const QUuid& channel_id)
	{
		// FIXME: isolate update to what changed, instead of re-exporting the whole resource
		Q_UNUSED(animation_id);
		Q_UNUSED(channel_id);
		m_animation_model->store_to_resource(m_resource);

		update_title();
	}

	void AnimationResourceWidget::save_triggered()
	{
		if (m_resource->is_modified() == false)
		{
			// Nothing to save
			return;
		}

		Core::Application& application = m_resource->get_application();
		Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
		if (resource_system.save_resource(m_resource) == false)
		{
			Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::save_triggered", "Failed to save resource");
			return;
		}

		update_title();
	}

	void AnimationResourceWidget::update_title()
	{
		QString title_text = QString("Animation Editor - %1").arg(get_label());
		if (m_resource->is_modified() == true)
		{
			title_text += " (*)";
		}
		setWindowTitle(title_text);
	}

	QString AnimationResourceWidget::get_label() const
	{
		Core::Application& application = m_resource->get_application();
		const VadonEditor::Model::ResourceInfo resource_info = m_resource->get_info();

		// Set label based on asset path
		QString resource_path;
		{
			// If nested resource, use the "path" from parents
			// FIXME: instead of the verbose UUIDs, use the property IDs/names
			const Core::AssetManager& asset_manager = application.get_asset_manager();

			const Model::Resource* current_resource = m_resource;
			QString asset_path;
			while (current_resource->get_owner())
			{
				// FIXME: replace this with the property name!
				QString resource_id_string = current_resource->get_info().id.toString();
				asset_path = asset_path.isEmpty() ? resource_id_string : QString("%1/%2").arg(resource_id_string).arg(asset_path);

				current_resource = current_resource->get_owner();
			}

			Model::ResourceSystem& resource_system = application.get_model_system().get_resource_system();
			const int resource_asset_id = resource_system.find_resource_asset_id(resource_info.id);

			if (resource_asset_id == Core::AssetInfo::c_invalid_file_id)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::get_label", "Cannot find resource asset");
				return resource_path;
			}

			const QModelIndex asset_index = asset_manager.find_asset_index(resource_asset_id);
			if (asset_index.isValid() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceEditor::get_label", "Cannot find asset");
				return resource_path;
			}

			const Core::AssetInfo resource_asset_info = asset_manager.get_asset_info(asset_index);

			resource_path = asset_path.isEmpty() == false ? QString("%1/%2").arg(resource_asset_info.path).arg(asset_path) : resource_asset_info.path;
		}

		const Core::DataSchema& data_schema = application.get_project_manager().get_project_data_schema();
		const Core::TypeData* type_data = data_schema.find_type_data(resource_info.type);

		QString type_name = type_data->find_metadata(::Vadon::Foundation::CommonTypeMetadata::NAME);

		if (type_name.isEmpty())
		{
			type_name = QString("Resource type %1").arg(resource_info.type.toString());
		}

		return QString("%1 (%2)").arg(resource_path).arg(type_name);
	}
}