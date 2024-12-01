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
		Vadon::Scene::ResourceHandle get_handle() const { return m_handle; }
		const Vadon::Scene::ResourceInfo& get_info() const { return m_info; }

		void set_path(const Vadon::Scene::ResourcePath& path);

		bool is_modified() const { return m_modified; }

		bool save();
		bool load();

		Vadon::Utilities::Variant get_property(std::string_view property_name) const;
		void edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle);

		bool initialize();
		void update_info();

		void notify_modified() { m_modified = true; }

		Core::Editor& m_editor;

		Vadon::Scene::ResourceHandle m_handle;
		Vadon::Scene::ResourceInfo m_info;
		bool m_modified;

		friend class ResourceSystem;
	};
}
#endif