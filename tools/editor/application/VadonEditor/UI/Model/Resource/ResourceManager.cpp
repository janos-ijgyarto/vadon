#include <VadonEditor/UI/Model/Resource/ResourceManager.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Asset/AssetManager.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <VadonEditor/UI/UISystem.hpp>
#include <VadonEditor/UI/MainWindow.hpp>
#include <VadonEditor/UI/Model/Resource/ResourceEditor.hpp>

namespace VadonEditor::UI
{
	void ResourceManager::asset_opened(const QModelIndex& index)
	{
		const Core::AssetInfo asset_info = m_application.get_asset_manager().get_asset_info(index);
		if (asset_info.type != Core::AssetType::RESOURCE)
		{
			return;
		}

		Model::ResourceSystem& resource_system = m_application.get_model_system().get_resource_system();
		const Model::ResourceInfo resource_info = resource_system.resource_info_by_asset_id(asset_info.id);
		Q_ASSERT_X(resource_info.is_valid() == true, "VadonEditor::UI::ResourceManager::asset_opened", "Cannot find resource");

		auto editor_it = m_resource_widgets.find(resource_info.id);
		if (editor_it == m_resource_widgets.end())
		{
			Model::Resource* resource = resource_system.get_resource(resource_info.id);
			ResourceEditor* new_editor = new ResourceEditor(resource);
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
}