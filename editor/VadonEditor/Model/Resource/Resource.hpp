#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <Vadon/Scene/Resource/Resource.hpp>
#include <Vadon/Utilities/TypeInfo/Reflection/Property.hpp>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	using ResourceID = Vadon::Scene::ResourceID;

	using EditorResourceID = uint32_t;

	class Resource
	{
	public:
		const ResourceID& get_id() const { return m_resource_id; }
		Vadon::Scene::ResourceHandle get_handle() const { return m_handle; }

		EditorResourceID get_editor_id() const { return m_editor_id; }

		bool is_modified() const { return m_modified; }
		void notify_modified() { m_modified = true; }
		void clear_modified() { m_modified = false; }

		const Vadon::Scene::ResourceInfo& get_info() const { return m_info; }

		std::string get_path() const;

		bool save();
		bool save_as(std::string_view path);
		bool load();

		void unload();

		bool is_loaded() const { return m_handle.is_valid(); }

		Vadon::Utilities::PropertyList get_properties() const;
		Vadon::Utilities::Variant get_property(std::string_view property_name) const;
		void edit_property(std::string_view property_name, const Vadon::Utilities::Variant& value);

		void add_embedded_resource(Resource* resource);
		bool is_embedded() const { return m_owner != nullptr; }
	private:
		Resource(Core::Editor& editor, ResourceID resource_id, EditorResourceID editor_id);

		void internal_load(Vadon::Scene::ResourceHandle resource_handle);

		Core::Editor& m_editor;

		ResourceID m_resource_id;
		Vadon::Scene::ResourceHandle m_handle;

		Vadon::Scene::ResourceInfo m_info;
		EditorResourceID m_editor_id;

		// NOTE: can keep pointers, with embedded resources either these are valid or the resource shouldn't exist!
		Resource* m_owner;
		std::vector<Resource*> m_embedded_resources;

		bool m_modified;

		friend class ResourceSystem;
	};
}
#endif