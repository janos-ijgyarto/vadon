#ifndef VADONEDITOR_MODEL_MODELSYSTEM_HPP
#define VADONEDITOR_MODEL_MODELSYSTEM_HPP
#include <memory>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class ResourceSystem;
	class SceneSystem;

	class ModelSystem
	{
	public:
		~ModelSystem();

		ResourceSystem& get_resource_system();
		SceneSystem& get_scene_system();
	private:
		ModelSystem(Core::Application& application);

		bool initialize();
		void project_loaded();
		void shutdown();

		void simulator_initialized();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Application;
	};
}
#endif