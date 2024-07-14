#ifndef VADONEDITOR_MODEL_MODELSYSTEM_HPP
#define VADONEDITOR_MODEL_MODELSYSTEM_HPP
#include <VadonEditor/Model/Module.hpp>
#include <Vadon/ECS/Entity/Entity.hpp>
#include <Vadon/ECS/Component/Component.hpp>
#include <Vadon/Utilities/Data/Variant.hpp>
#include <memory>
#include <functional>
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
	class Entity;
	class SceneTree;

	class ModelSystem : public ModelSystemBase<ModelSystem>
	{
	public:
		~ModelSystem();

		VADONEDITOR_API Vadon::ECS::World& get_ecs_world();
		SceneTree& get_scene_tree();

		// FIXME: have a more flexible approach, where this systems runs ECS systems tagged to run in the editor?
		VADONEDITOR_API void add_callback(std::function<void()> callback);
	private:
		ModelSystem(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif