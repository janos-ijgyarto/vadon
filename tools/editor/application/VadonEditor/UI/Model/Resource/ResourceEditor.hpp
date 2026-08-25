#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEEDITOR_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEEDITOR_HPP
#include <VadonEditor/UI/Model/Resource/ui_ResourceEditor.h>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	class Resource;
}
namespace VadonEditor::UI
{
	class PropertyWidget;
	class ResourceEditor : public QWidget
	{
		Q_OBJECT
	public:
		ResourceEditor(Model::Resource* resource, QWidget* parent = nullptr, Qt::WindowType type = Qt::WindowType::Widget);
		bool initialize();

		const Model::Resource* get_resource() const { return m_resource; }

		QString get_label() const;

		void set_read_only(bool read_only);
	signals:
		void resource_property_edited(const QUuid& property_id);
	private slots:
		void internal_property_edited(const QUuid& property_id);
		void save_triggered();
		void reload_triggered();
	private:
		void update_title();

		PropertyWidget* find_property_widget(const QUuid& property_id) const;
		
		Model::Resource* m_resource;

		Ui::ResourceEditor m_ui;
	};
}
#endif