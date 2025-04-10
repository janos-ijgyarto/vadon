#ifndef VADONEDITOR_MODEL_MODELSYSTEM_HPP
#define VADONEDITOR_MODEL_MODELSYSTEM_HPP
#include <VadonEditor/Model/Module.hpp>
#include <memory>
namespace Vadon::ECS
{
	class World;
}
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class ResourceSystem;
	class SceneSystem;

	class ModelSystem : public ModelSystemBase<ModelSystem>
	{
	public:
		~ModelSystem();

		VADONEDITOR_API Vadon::ECS::World& get_ecs_world();
		VADONEDITOR_API ResourceSystem& get_resource_system();
		VADONEDITOR_API SceneSystem& get_scene_system();
	private:
		ModelSystem(Core::Editor& editor);

		bool initialize();
		bool load_project();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif