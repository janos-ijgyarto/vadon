#ifndef VADONEDITOR_UI_SCHEMAEDITOR_HPP
#define VADONEDITOR_UI_SCHEMAEDITOR_HPP
#include <QDialog>
#include <VadonEditor/Forms/ui_SchemaEditor.h>
#include <QAbstractTableModel>
#include <QUuid>
namespace VadonEditor::UI
{
	struct PropertyData
	{
		QUuid id;
		QUuid type;
		QString name;
	};

	struct TypeData
	{
		QUuid id;
		QString name;
		QList<PropertyData> properties;
	};

	struct Schema
	{
		QList<TypeData> types;
	};

	class TypeListModel : public QAbstractListModel
	{
		Q_OBJECT
	public:
		TypeListModel();

		void schema_loaded(const Schema* schema);

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	private:
		const Schema* m_schema;
	};

	class TypeInfoModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		explicit TypeInfoModel(QObject* parent = nullptr);

		void select_type(const TypeData* type_data);

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	private:
		const TypeData* m_selected_type;
	};

	class SchemaEditor : public QDialog
	{
		Q_OBJECT
	public:
		SchemaEditor(QWidget* parent);
	private slots:
		void loadClicked();
		void type_clicked(const QModelIndex& index);
	private:
		Ui::SchemaEditor m_ui;

		Schema m_schema;
		TypeListModel m_type_list;
		TypeInfoModel m_type_info;
	};
}
#endif