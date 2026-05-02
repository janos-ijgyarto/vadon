#ifndef VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#define VADONEDITOR_UI_MODEL_SCENE_ENTITYEDITOR_HPP
#include <VadonEditor/UI/Model/Scene/ui_EntityEditor.h>
namespace VadonEditor::Model
{
	class Entity;
	struct Component;
}
namespace VadonEditor::UI
{
	class EntityEditor : public QWidget
	{
		Q_OBJECT
	public:
	signals:
		void component_property_edited(const QUuid& entity_id, const QUuid& component_id, const QUuid& property_id);
	private slots:
		void name_changed(const QString& text);
		void add_component_clicked();
		void new_component_selected(const QUuid& component_type);

		void internal_component_property_edited(const QUuid& component_id, const QUuid& property_id);
		void component_remove_requested(const QUuid& component_id);
	private:
		EntityEditor(Model::Entity* entity, QWidget* parent = nullptr);

		bool initialize();
		bool update_title();

		bool internal_add_component_widget(Model::Component* component);

		bool request_close();

		Ui::EntityEditor m_ui;
		Model::Entity* m_entity;

		friend class SceneManager;
	};
}
#endif