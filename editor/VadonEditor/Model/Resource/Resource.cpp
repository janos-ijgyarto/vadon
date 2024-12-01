#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <Vadon/Core/File/FileSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/Serialization/Serializer.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

namespace VadonEditor::Model
{
	void Resource::set_path(const Vadon::Scene::ResourcePath& path)
	{
		// TODO: should we check here whether we are trying to overwrite an existing file?
		// TODO2: validate extension?
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		resource_system.set_resource_path(m_handle, path);

		notify_modified();
		update_info();
	}

	bool Resource::save()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.save_resource(m_handle) == false)
		{
			resource_system.log_error("Unable to save resource!\n");
			return false;
		}

		m_modified = false;
		return true;
	}

	bool Resource::load()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		if (resource_system.load_resource(m_handle) == false)
		{
			resource_system.log_error("Unable to load resource!\n");
			return false;
		}

		update_info();
		return true;
	}

	Vadon::Utilities::Variant Resource::get_property(std::string_view property_name) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_handle);
		return Vadon::Utilities::TypeRegistry::get_property(resource_base, m_info.type_id, property_name);
	}

	void Resource::edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_handle);
		Vadon::Utilities::TypeRegistry::set_property(resource_base, m_info.type_id, property_name, value);

		notify_modified();
	}

	Resource::Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle)
		: m_editor(editor)
		, m_handle(resource_handle)
		, m_modified(false)
	{

	}

	bool Resource::initialize()
	{
		// TODO: anything else?
		update_info();
		return true;
	}

	void Resource::update_info()
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		m_info = resource_system.get_resource_info(m_handle);
	}
}