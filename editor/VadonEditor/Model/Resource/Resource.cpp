#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

namespace VadonEditor::Model
{
	Vadon::Scene::ResourceInfo VadonEditor::Model::Resource::get_info() const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		return resource_system.get_resource_info(m_handle);
	}

	void Resource::set_path(const ResourcePath& path)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		resource_system.set_resource_path(m_handle, path);
	}

	bool Resource::save()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.save_resource(m_handle) == false)
		{
			resource_system.log_error("Editor resource: failed to save resource!\n");
			return false;
		}

		clear_modified();
		return true;
	}

	bool Resource::load()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.is_resource_loaded(m_handle) == true)
		{
			return true;
		}

		if (resource_system.load_resource(m_handle) == false)
		{
			resource_system.log_error("Editor resource: failed to load resource!\n");
			return false;
		}

		clear_modified();
		return true;
	}

	bool Resource::is_loaded() const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		return resource_system.is_resource_loaded(m_handle);
	}

	Vadon::Utilities::Variant Resource::get_property(std::string_view property_name) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_handle);
		return Vadon::Utilities::TypeRegistry::get_property(resource_base, get_info().type_id, property_name);
	}

	void Resource::edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_handle);
		Vadon::Utilities::TypeRegistry::set_property(resource_base, get_info().type_id, property_name, value);

		notify_modified();
	}

	Resource::Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle)
		: m_editor(editor)
		, m_handle(resource_handle)
		, m_modified(false)
	{

	}
}