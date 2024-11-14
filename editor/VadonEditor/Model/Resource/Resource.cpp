#include <VadonEditor/Model/Resource/Resource.hpp>

#include <VadonEditor/Core/Editor.hpp>

#include <Vadon/Scene/Resource/ResourceSystem.hpp>

#include <Vadon/Utilities/TypeInfo/Registry/Registry.hpp>

namespace VadonEditor::Model
{
	Vadon::Utilities::Variant Resource::get_property(std::string_view property_name) const
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_resource_handle);
		const Vadon::Utilities::TypeID resource_type_id = resource_system.get_resource_type_id(m_resource_handle);

		return Vadon::Utilities::TypeRegistry::get_property(resource_base, resource_type_id, property_name);
	}

	void Resource::edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value)
	{
		Vadon::Scene::ResourceSystem& resource_system = m_editor.get_engine_core().get_system<Vadon::Scene::ResourceSystem>();

		Vadon::Scene::ResourceBase* resource_base = resource_system.get_resource_base(m_resource_handle);
		const Vadon::Utilities::TypeID resource_type_id = resource_system.get_resource_type_id(m_resource_handle);

		Vadon::Utilities::TypeRegistry::set_property(resource_base, resource_type_id, property_name, value);
	}

	Resource::Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle)
		: m_editor(editor)
		, m_resource_handle(resource_handle)
	{

	}

	void Resource::update_name()
	{
		m_name = std::get<std::string>(get_property("name"));
	}
}