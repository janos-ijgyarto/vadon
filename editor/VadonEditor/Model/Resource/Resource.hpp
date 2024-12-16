#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Registry/Property.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	using ResourcePath = Vadon::Scene::ResourcePath;

	using EditorResourceID = uint32_t;

	class Resource
	{
	public:
		Vadon::Scene::ResourceHandle get_handle() const { return m_handle; }
		Vadon::Scene::ResourceInfo get_info() const;

		EditorResourceID get_editor_id() const { return m_id; }

		void set_path(const ResourcePath& path);

		bool is_modified() const { return m_modified; }
		void notify_modified() { m_modified = true; }
		void clear_modified() { m_modified = false; }

		bool load();
		bool save();

		bool is_loaded() const;

		Vadon::Utilities::PropertyList get_properties() const;
		Vadon::Utilities::Variant get_property(std::string_view property_name) const;
		void edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value);
	private:
		Resource(Core::Editor& editor, Vadon::Scene::ResourceHandle resource_handle, EditorResourceID id);

		Core::Editor& m_editor;

		Vadon::Scene::ResourceHandle m_handle;
		EditorResourceID m_id;
		bool m_modified;

		friend class ResourceSystem;
	};
}
#endif