#ifndef VADONEDITOR_VIEW_VIEWSYSTEM_HPP
#define VADONEDITOR_VIEW_VIEWSYSTEM_HPP
#include <VadonEditor/View/Module.hpp>
#include <memory>
namespace VadonEditor::Core
{
	class Editor;
}
namespace VadonEditor::Model
{
	class Entity;
	class Scene;
}
namespace VadonEditor::View
{
	class ViewModel
	{
	public:
		Model::Scene* get_active_scene() const { return m_active_scene; }
		void set_active_scene(Model::Scene* scene);

		Model::Entity* get_active_entity() const { return m_active_entity; }
		void set_active_entity(Model::Entity*);
	private:
		VADONEDITOR_API ViewModel(Core::Editor& editor);

		Core::Editor& m_editor;

		Model::Scene* m_active_scene;
		Model::Entity* m_active_entity;

		friend class ViewSystem;
	};

	class ViewSystem : public ViewSystemBase<ViewSystem>
	{
	public:
		~ViewSystem();

		ViewModel& get_view_model();
	private:
		ViewSystem(Core::Editor& editor);

		bool initialize();
		void update();

		struct Internal;
		std::unique_ptr<Internal> m_internal;

		friend Core::Editor;
	};
}
#endif