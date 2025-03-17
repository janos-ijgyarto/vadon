#ifndef VADONDEMO_VIEW_EDITORUI_HPP
#define VADONDEMO_VIEW_EDITORUI_HPP
#include <Vadon/ECS/Entity/Entity.hpp>
namespace VadonDemo::Core
{
	class Editor;
}
namespace VadonDemo::UI
{
	class EditorUI
	{
	public:
	private:
		EditorUI(Core::Editor& editor);
		
		bool initialize();
		void update();

		void init_entity(Vadon::ECS::EntityHandle entity);
		void update_entity(Vadon::ECS::EntityHandle entity);
		void remove_entity(Vadon::ECS::EntityHandle entity);

		Core::Editor& m_editor;

		friend Core::Editor;
	};
}
#endif