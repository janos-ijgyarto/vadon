#ifndef VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#include <VadonEditor/UI/Model/Scene/ui_EntityEditor.h>
namespace VadonEditor::Model
{
	class Entity;
	class Component;
	class Scene;
}
namespace VadonEditor::UI
{
	class EntityEditor : public QWidget
	{
		Q_OBJECT
	public:
	protected:
		void closeEvent(QCloseEvent* event) override;
	private slots:
		void internal_name_changed(const QString& text);
		void add_component_clicked();
		void new_component_selected(const QUuid& component_type);

		void component_remove_requested(const QUuid& component_id);
	private:
		EntityEditor(Model::Scene* scene, Model::Entity* entity, QWidget* parent = nullptr, Qt::WindowType type = Qt::WindowType::Widget);

		bool initialize();
		bool update_title();

		bool internal_add_component_widget(Model::Component* component);

		void store_entity_data();

		Ui::EntityEditor m_ui;
		Model::Scene* m_scene;
		Model::Entity* m_entity;

		friend class SceneTree;
	};
}
#endif