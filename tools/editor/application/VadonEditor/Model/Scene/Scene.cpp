#include <VadonEditor/Model/Scene/Scene.hpp>

#include <VadonEditor/Core/Application.hpp>
#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

#include <Vadon/Foundation/Model/Scene/Scene.hpp>

namespace VadonEditor::Model
{
	bool Scene::is_scene_base_of_type(Core::Application& application, const QUuid& type_id)
	{
		return application.get_project_manager().get_project_data_schema().is_base_of(get_scene_type_uuid(), type_id);
	}

	QUuid Scene::get_scene_type_uuid()
	{
		return Utilities::vadon_uuid_string_to_qt_uuid(::Vadon::Foundation::SceneSchema::c_type_uuid);
	}
}