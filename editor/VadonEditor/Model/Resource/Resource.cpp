#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <VadonEditor/Model/ModelSystem.hpp>
#include <VadonEditor/Model/Resource/ResourceSystem.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Registry.hpp>

namespace VadonEditor::Model
{
	ResourcePath Resource::get_path() const
	{
		ResourceSystem& editor_resource_system = m_editor.get_system<ModelSystem>().get_resource_system();
		return editor_resource_system.get_database().find_resource_info(m_resource_id).path;
	}

	bool Resource::save()
	{
		if (is_loaded() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource: cannot save resource that is not loaded!\n");
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

	bool Resource::save_as(const ResourcePath& path)
	{
		if (is_loaded() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource: cannot save resource that is not loaded!\n");
			return true;
		}

		EditorResourceDatabase& editor_resource_db = m_editor.get_system<ModelSystem>().get_resource_system().get_database();
		return editor_resource_db.save_resource_as(m_resource_id, path);
	}

	bool Resource::load()
	{
		if (is_loaded() == true)
		{
			// Nothing to do
			return true;
		}

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		Vadon::Scene::ResourceHandle resource_handle = resource_system.load_resource(m_resource_id);

		if (resource_handle.is_valid() == false)
		{
			resource_system.log_error("Editor resource: failed to load resource!\n");
			return false;
		}

		internal_load(resource_handle);
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
		if (is_loaded() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource: cannot get properties of unloaded resource!\n");
		}

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		return Vadon::Utilities::TypeRegistry::get_properties(resource_base, get_info().type_id);
	}

	Vadon::Utilities::Variant Resource::get_property(std::string_view property_name) const
	{
		if (is_loaded() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource: cannot get properties of unloaded resource!\n");
		}

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		return Vadon::Utilities::TypeRegistry::get_property(resource_base, get_info().type_id, property_name);
	}

	void Resource::edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		if (is_loaded() == false)
		{
			Vadon::Core::Logger::log_error("Editor resource: cannot edit properties of unloaded resource!\n");
		}

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::Resource* resource_base = resource_system.get_base_resource(m_handle);
		Vadon::Utilities::TypeRegistry::set_property(resource_base, get_info().type_id, property_name, value);

		// Notify Editor resource system
		m_editor.get_system<ModelSystem>().get_resource_system().resource_edited(*this);

		notify_modified();
	}

	Resource::Resource(Core::Editor& editor, ResourceID resource_id, EditorResourceID editor_id)
		: m_editor(editor)
		, m_resource_id(resource_id)
		, m_editor_id(editor_id)
		, m_modified(false)
	{
	}

	void Resource::internal_load(Vadon::Scene::ResourceHandle resource_handle)
	{
		m_handle = resource_handle;

		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();
		m_info = resource_system.get_resource_info(resource_handle);

		clear_modified();
	}
}