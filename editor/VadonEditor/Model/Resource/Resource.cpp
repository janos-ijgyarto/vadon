#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

namespace VadonEditor::Model
{
	bool Resource::save()
	{
		if (is_loaded() == false)
		{
			// Nothing to do
			// TODO: should this be an error?
			return true;
		}

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
		if (is_loaded() == true)
		{
			// Nothing to do
			return true;
		}
		
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		m_handle = resource_system.load_resource(m_resource_id);

		if (m_handle.is_valid() == false)
		{
			resource_system.log_error("Editor resource: failed to load resource!\n");
			return false;
		}

		clear_modified();
		return true;
	}

	void Resource::unload()
	{
		if (is_loaded() == false)
		{
			return;
		}

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		resource_system.remove_resource(m_handle);

		m_handle.invalidate();
	}

	Vadon::Utilities::PropertyList Resource::get_properties() const
	{
		// TODO: ensure that resource is loaded!
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		return Vadon::Utilities::TypeRegistry::get_properties(resource_base, get_info().type_id);
	}

	Vadon::Utilities::Variant Resource::get_property(std::string_view property_name) const
	{
		// TODO: ensure that resource is loaded!
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		return Vadon::Utilities::TypeRegistry::get_property(resource_base, get_info().type_id, property_name);
	}

	void Resource::edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		// TODO: ensure that resource is loaded!
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		Vadon::Utilities::TypeRegistry::set_property(resource_base, get_info().type_id, property_name, value);

		notify_modified();
	}

	Resource::Resource(Core::Editor& editor, ResourceID resource_id, EditorResourceID editor_id)
		: m_editor(editor)
		, m_resource_id(resource_id)
		, m_editor_id(editor_id)
		, m_modified(false)
	{
	}
}