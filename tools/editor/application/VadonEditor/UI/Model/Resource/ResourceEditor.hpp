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
	class ResourceEditor : public QWidget
	{
		Q_OBJECT
	public:
		ResourceEditor(Model::Resource* resource, QWidget* parent = nullptr, Qt::WindowType type = Qt::WindowType::Widget);
		bool initialize();

		const Model::Resource* get_resource() const { return m_resource; }

		QString get_label() const;

		void set_read_only(bool read_only);

		bool request_close();
	signals:
		void resource_property_edited(const QUuid& property_id);
	private slots:
		void internal_property_edited(const QUuid& property_id);
	private:
		void set_modified() { m_modified = true; update_title(); }
		void clear_modified() { m_modified = false; update_title(); }

		void update_title();
		
		Model::Resource* m_resource;
		bool m_modified; // TODO: create more advanced system of "versioning" for changes (undo/redo, etc.)

		Ui::ResourceEditor m_ui;
	};
}
#endif