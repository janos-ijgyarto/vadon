#ifndef VADONEDITOR_UI_MODEL_PROPERTY_RESOURCE_HPP
#define VADONEDITOR_UI_MODEL_PROPERTY_RESOURCE_HPP
#include <VadonEditor/UI/Model/Property/Property.hpp>
#include <VadonEditor/UI/Model/Property/ui_PropertyResource.h>
namespace VadonEditor::UI
{
	class PropertyResource : public PropertyWidget
	{
		Q_OBJECT
	public:
		PropertyResource(const QUuid& id, Model::Resource* resource, Model::Resource* owner_resource, const QUuid& base_type, QWidget* parent);

		void set_read_only(bool read_only) override;
	private slots:
		void resource_property_value_changed(const QUuid& id);

		void new_triggered();
		void load_triggered();
		void clear_triggered();

		void new_resource_type_selected(const QUuid& resource_type);
		void resource_asset_opened(const QString& resource_path);
	private:
		QUuid get_resource_id() const;
		Model::Resource* find_resource() const;

		void generate_resource_widgets();
		void clear_resource_widgets();

		Ui::PropertyResource m_ui;

		Model::Resource* m_resource;
		Model::Resource* m_owner_resource;
		QUuid m_base_type;
		bool m_read_only;
	};
}
#endif