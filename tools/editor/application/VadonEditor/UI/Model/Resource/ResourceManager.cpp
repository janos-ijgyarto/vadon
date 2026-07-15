#include <VadonEditor/UI/Model/Resource/ResourceManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>
#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>

#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

namespace VadonEditor::UI
{
	UnsavedResourceAssetFilter::UnsavedResourceAssetFilter(Core::Application& application, QObject* parent)
		: QSortFilterProxyModel(parent)
		, m_application(application)
	{
	}

	bool UnsavedResourceAssetFilter::initialize()
	{
		const Core::DataSchema& data_schema = m_application.get_project_manager().get_project_data_schema();

		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		QList<Model::Resource*> active_resources = resource_system.get_all_resources();
		for (Model::Resource* current_resource : active_resources)
		{
			if (current_resource->is_embedded() == true)
			{
				continue;
			}

			if (data_schema.is_base_of(Model::Scene::get_scene_type_uuid(), current_resource->get_info().type) == true)
			{
				// Skip scenes
				continue;
			}

			if (current_resource->is_modified() == true)
			{
				const int asset_id = resource_system.find_resource_asset_id(current_resource->get_info().id);
				Q_ASSERT_X(asset_id != Core::AssetInfo::c_invalid_file_id, "VadonEditor::UI::UnsavedResourceAssetFilter::initialize", "Cannot find resource asset");
				m_unsaved_resource_asset_ids.push_back(asset_id);
			}
		}

		if (m_unsaved_resource_asset_ids.isEmpty() == true)
		{
			return false;
		}

		setSourceModel(&m_application.get_asset_manager().get_model());
		return true;
	}

	bool UnsavedResourceAssetFilter::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
	{
		QModelIndex source_index = sourceModel()->index(source_row, 0, source_parent);

		Core::AssetManager& asset_manager = m_application.get_asset_manager();
		const Core::AssetInfo asset_info = asset_manager.get_asset_info(source_index);

		if (m_unsaved_resource_asset_ids.indexOf(asset_info.id) != -1)
		{
			return true;
		}

		// Quick check that the base filtering doesn't reject this row
		if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent) == false)
		{
			return false;
		}

		if (sourceModel()->hasChildren(source_index))
		{
			for (int child_index = 0; child_index < sourceModel()->rowCount(source_index); ++child_index)
			{
				if (filterAcceptsRow(child_index, source_index) == true)
				{
					return true;
				}
			}
		}

		return false;
	}

	UnsavedResourceMessageBox::UnsavedResourceMessageBox(QAbstractItemModel* asset_model, QWidget* parent)
		: AssetMessageBox(asset_model, parent)
	{
		setWindowTitle("Save Resources");

		m_ui.mainLayout->insertWidget(0, new QLabel("The following resources have unsaved changes. Would you like to save before closing?"));
	}

	void ResourceManager::asset_opened(const QModelIndex& index)
	{
		const Core::AssetInfo asset_info = m_application.get_asset_manager().get_asset_info(index);
		if (asset_info.type != Core::AssetType::RESOURCE)
		{
			// Make sure we only try to open Resource assets
			return;
		}

		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		const Model::ResourceInfo resource_info = resource_system.resource_info_by_asset_id(asset_info.id);
		Q_ASSERT_X(resource_info.is_valid() == true, "VadonEditor::UI::ResourceManager::asset_opened", "Cannot find resource");

		auto editor_it = m_resource_widgets.find(resource_info.id);
		if (editor_it == m_resource_widgets.end())
		{
			Model::Resource* resource = resource_system.get_resource(resource_info.id);
			resource->open();

			ResourceEditor* new_editor = new ResourceEditor(resource, m_application.get_ui_system().get_main_window(), Qt::WindowType::Window);
			if (new_editor->initialize() == false)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceManager::asset_opened", "Failed to initialize Resource editor!");
				delete new_editor;
				return;
			}

			m_resource_widgets.insert(resource_info.id, new_editor);
			m_widget_reverse_lookup.insert(new_editor, resource_info.id);

			connect(new_editor, &QObject::destroyed, this, &ResourceManager::resource_widget_removed);

			new_editor->show();
		}
		else
		{
			QWidget* editor_widget = editor_it.value();
			editor_widget->setWindowState((editor_widget->windowState() & ~Qt::WindowState::WindowMinimized) | Qt::WindowState::WindowActive);
			editor_widget->raise();
			editor_widget->activateWindow();
		}
	}

	void ResourceManager::resource_widget_removed(QObject* widget_obj)
	{
		QWidget* editor_widget = qobject_cast<QWidget*>(widget_obj);
		auto editor_uuid_it = m_widget_reverse_lookup.find(editor_widget);
		if (editor_uuid_it != m_widget_reverse_lookup.end())
		{
			m_resource_widgets.remove(editor_uuid_it.value());
			m_widget_reverse_lookup.erase(editor_uuid_it);
		}
	}

	ResourceManager::ResourceManager(Core::Application& application)
		: m_application(application)
	{

	}

	bool ResourceManager::initialize()
	{
		connect(&m_application.get_asset_manager(), &Core::AssetManager::asset_opened, this, &ResourceManager::asset_opened);

		return true;
	}

	void ResourceManager::shutdown()
	{
	}

	bool ResourceManager::request_close()
	{
		UnsavedResourceAssetFilter* asset_filter = new UnsavedResourceAssetFilter(m_application);
		if(asset_filter->initialize() == true)
		{
			UnsavedResourceMessageBox message_box(asset_filter, m_application.get_ui_system().get_main_window());
			
			const int result = message_box.exec();
			switch (result)
			{
			case QDialog::DialogCode::Accepted:
			{
				Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
				resource_system.save_all_resources();
			}
			break;
			case QDialog::DialogCode::Rejected:
			{
				// Cancel closing the editor
				return false;
			}
			break;
			}
		}

		// Close all widgets
		for (auto resource_widget_it = m_resource_widgets.begin(); resource_widget_it != m_resource_widgets.end(); ++resource_widget_it)
		{
			ResourceEditor* resource_widget = qobject_cast<ResourceEditor*>(resource_widget_it.value());
			if (resource_widget == nullptr)
			{
				Q_ASSERT_X(false, "VadonEditor::UI::ResourceManager::request_close", "Invalid widget in lookup");
				continue;
			}

			resource_widget->close();
		}

		return true;
	}
	
	void ResourceManager::force_close()
	{
		// Clear reverse lookup so we can ignore the signal from when the widgets are destroyed on close
		m_widget_reverse_lookup.clear();

		for (auto resource_widget_it = m_resource_widgets.begin(); resource_widget_it != m_resource_widgets.end(); ++resource_widget_it)
		{
			resource_widget_it.value()->close();
		}

		m_resource_widgets.clear();
	}
}