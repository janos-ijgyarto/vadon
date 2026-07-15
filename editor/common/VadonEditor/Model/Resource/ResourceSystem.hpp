#ifndef VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#define VADONEDITOR_MODEL_RESOURCE_RESOURCESYSTEM_HPP
#include <VadonEditor/VadonEditor.hpp>

#include <VadonEditor/Model/Resource/Database.hpp>
#include <VadonEditor/Model/Resource/Event.hpp>
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
		using EventCallback = std::function<void(const ResourceEvent&)>;

		~ResourceSystem();

		VADONEDITOR_API const Resource* find_resource(const Vadon::Model::ResourceID& resource_id) const;
		Resource* find_resource(const Vadon::Model::ResourceID& resource_id) { return const_cast<Resource*>(std::as_const(*this).find_resource(resource_id)); }

		Resource* get_resource(const Vadon::Model::ResourceID& resource_id);

		VADONEDITOR_API void register_event_callback(EventCallback callback);
		VADONEDITOR_API void broadcast_resource_event(const ResourceEvent& event);
	private:
		ResourceSystem(Core::Editor& editor);

		bool initialize();
		void shutdown();

		bool project_loaded();

		void process_message(const char* data, size_t size);

		void internal_add_resource(Resource* resource);
		void internal_remove_resource(Resource* resource, bool force_remove = false);

		void reload_resource(Resource* resource);

		Core::Editor& m_editor;

		ResourceDatabase m_database;

		std::unordered_map<Vadon::Model::ResourceID, Resource*> m_resource_lookup;

		std::vector<EventCallback> m_event_callbacks;

		friend Core::Editor;
	};
}
#endif