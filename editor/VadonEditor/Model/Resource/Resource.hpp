#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Resource
	{
	public:
		const std::string& get_name() const { return m_name; }

		Vadon::Utilities::Variant get_property(std::string_view property_name) const;
		void edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle);
		void update_name();

		Core::Editor& m_editor;

		Vadon::Scene::ResourceHandle m_resource_handle;
		std::string m_name;

		friend class ResourceSystem;
	};
}
#endif