#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>

#include <VadonEditor/Model/Resource/Database.hpp>
#include <VadonEditor/Model/Resource/Resource.hpp>

#include <functional>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class ResourceSystem
	{
	public:
		// FIXME: extend this so we can get info on what was modified!
		using EditCallback = std::function<void(Vadon::Model::ResourceID, ::Vadon::Foundation::UUID)>;

		~ResourceSystem();

		VADONEDITOR_API const Resource* find_resource(const Vadon::Model::ResourceID& resource_id) const;
		Resource* find_resource(const Vadon::Model::ResourceID& resource_id) { return const_cast<Resource*>(std::as_const(*this).find_resource(resource_id)); }

		Resource* get_resource(const Vadon::Model::ResourceID& resource_id);

		VADONEDITOR_API void register_edit_callback(EditCallback callback);
		VADONEDITOR_API void resource_property_edited(const Vadon::Model::ResourceID& resource_id, const ::Vadon::Foundation::UUID& property_uuid);
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();
		void shutdown();

		bool project_loaded();

		void process_message(const char* data, size_t size);

		void internal_add_resource(Resource* resource);
		void internal_remove_resource(Resource* resource);

		Core::Editor& m_editor;

		ResourceDatabase m_database;

		std::unordered_map<Vadon::Model::ResourceID, Resource*> m_resource_lookup;

		std::vector<EditCallback> m_edit_callbacks;

		friend Core::Editor;
	};
}
#endif