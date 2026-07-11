#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCE_HPP
#include <VadonEditor/VadonEditor.hpp>
#include <Vadon/Model/Resource/Resource.hpp>

#include <Vadon/Foundation/Editor/Network/Message/Model.hpp>
namespace Vadon::Utilities
{
	class Serializer;
}
namespace VadonEditor::Core
{
	class Editor;
	struct Property;
}
namespace VadonEditor::Model
{
	class Resource
	{
	public:
		// TODO: implement reference counting
		// Each time something is loaded that references this resource
		// add a reference
		// Allows us to unload when nothing references it anymore
		const Vadon::Model::ResourceID& get_id() const { return m_id; }
		const Vadon::Model::ResourceInfo& get_info() const { return m_info; }

		bool is_loaded() const { return m_handle.is_valid(); }

		Resource* find_embedded_resource(const Vadon::Model::ResourceID& resource_id) const;
	private:
		Resource(Core::Editor& editor, const Vadon::Model::ResourceID& id);

		bool internal_load();
		void load_property_data(const ::Vadon::Foundation::EditorModelMessageResourcePropertyEdited& resource_property_message, const char* data);
		bool load_property_data(Vadon::Utilities::Serializer& serializer);

		Resource* add_embedded_resource(const Vadon::Model::ResourceID& id, const ::Vadon::Foundation::UUID& type_id);
		void remove_embedded_resource(const Vadon::Model::ResourceID& id);

		Core::Editor& m_editor;

		const Vadon::Model::ResourceID m_id;
		Vadon::Model::ResourceInfo m_info;
		Vadon::Model::ResourceHandle m_handle;

		Resource* m_owner;
		std::vector<Resource*> m_embedded_resources;

		friend class ResourceSystem;
	};
}
#endif