#ifndef VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#include <VadonEditor/UI/Model/Scene/ui_EntityEditor.h>
namespace VadonEditor::Model
{
	class Entity;
}
namespace VadonEditor::UI
{
	class EntityEditor : public QWidget
	{
		Q_OBJECT
	public:
	private slots:
		void name_changed(const QString& text);
		void add_component_clicked();
	private:
		EntityEditor(Model::Entity* entity, QWidget* parent = nullptr);

		bool initialize();
		bool update_title();

		Ui::EntityEditor m_ui;
		Model::Entity* m_entity;

		friend class SceneManager;
	};
}
#endif