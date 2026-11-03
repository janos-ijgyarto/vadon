#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEDIALOG_HPP
#include <VadonEditor/UI/Model/Object/ObjectDialog.hpp>
#include <VadonEditor/UI/Model/Resource/ui_NewResourceDialog.h>
#include <VadonEditor/UI/Model/Resource/ui_SelectResourceDialog.h>
#include <QSortFilterProxyModel>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::Model
{
	struct ResourceInfo;
}
namespace VadonEditor::UI
{
	class SelectResourceTypeDialog : public NewObjectDialog
	{
		Q_OBJECT
	public:
		SelectResourceTypeDialog(Core::Application& application, const QUuid& base_type, QWidget* parent = nullptr);
	};

	class NewResourceDialog : public QDialog
	{
		Q_OBJECT
	public:
		NewResourceDialog(Core::Application& application, const QUuid& base_type, const QModelIndex& root_asset = QModelIndex(), QWidget* parent = nullptr);

		void accept() override;
	private slots:
		void select_type_clicked();
		void resource_type_selected(const QUuid& type_uuid);

		void browse_file_path_clicked();
		void file_path_selected(const QString& asset_path);
	private:
		void update_controls();

		Core::Application& m_application;
		Ui::NewResourceDialog m_ui;

		QUuid m_base_type;
		QModelIndex m_root_asset;

		QUuid m_new_resource_type;
		QString m_new_resource_path;
	};

	class ResourceAssetFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		ResourceAssetFilterModel(Core::Application& application, const QUuid& resource_type, QObject* parent = nullptr);

		const QUuid& get_resource_type() const { return m_resource_type; }
	protected:
		bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
	private:
		Core::Application& m_application;
		QUuid m_resource_type;
	};

	class SelectResourceDialog : public QDialog
	{
		Q_OBJECT
	public:
		SelectResourceDialog(Core::Application& application, const QUuid& resource_type, QWidget* dialog_parent = nullptr);
	signals:
		void resource_asset_selected(const QUuid& resource_id);
	private slots:
		void filter_text_changed(const QString& text);
		void tree_item_clicked(const QModelIndex& index);
		void tree_item_double_clicked(const QModelIndex& index);
		void selection_accepted();
	private:
		void resource_selected(const QModelIndex& index);

		void update_buttons();
		bool is_compatible_item(const Model::ResourceInfo& resource_info) const;
		Model::ResourceInfo get_resource_info(const QModelIndex& index) const;

		Ui::SelectResourceDialog m_ui;

		Core::Application& m_application;
		ResourceAssetFilterModel m_filter_model;
		QModelIndex m_selection;
	};
}
#endif