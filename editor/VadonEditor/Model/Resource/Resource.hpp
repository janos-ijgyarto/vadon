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
	using ResourcePath = Vadon::Scene::ResourcePath;

	class Resource
	{
	public:
		Vadon::Scene::ResourceHandle get_handle() const { return m_handle; }
		Vadon::Scene::ResourceInfo get_info() const;

		void set_path(const ResourcePath& path);

		bool is_modified() const { return m_modified; }
		void notify_modified() { m_modified = true; }
		void clear_modified() { m_modified = false; }

		bool load();
		bool save();

		bool is_loaded() const;

		Vadon::Utilities::Variant get_property(std::string_view property_name) const;
		void edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle);

		Core::Editor& m_editor;

		Vadon::Scene::ResourceHandle m_handle;
		bool m_modified;

		friend class ResourceSystem;
	};
}
#endif