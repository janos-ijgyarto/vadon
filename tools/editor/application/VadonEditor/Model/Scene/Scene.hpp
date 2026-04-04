#ifndef VADONEDITOR_MODEL_SCENE_SCENE_HPP
#define VADONEDITOR_MODEL_SCENE_SCENE_HPP
#include <VadonEditor/Model/Scene/Entity.hpp>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Resource;
	class Scene
	{
	public:
		static bool is_scene_base_of_type(VadonEditor::Core::Application& application, const QUuid& type_id);
		static QUuid get_scene_type_uuid();
	private:
		Resource* m_resource;
		Entity m_root_entity;
	};
}
#endif