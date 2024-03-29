#ifndef VADONEDITOR_MODEL_MODELSYSTEM_HPP
#define VADONEDITOR_MODEL_MODELSYSTEM_HPP
#include <VadonEditor/Model/Module.hpp>
#include <Vadon/Core/Object/ClassInfo.hpp>
#include <memory>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class SceneTree;

	class ModelSystem : public ModelSystemBase<ModelSystem>
	{
	public:
		~ModelSystem();

		SceneTree& get_scene_tree();

		Vadon::Core::ObjectClassInfoList get_node_type_list() const;		
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