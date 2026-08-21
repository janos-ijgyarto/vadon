#ifndef VADONEDITOR_UI_PROJECT_ASSET_EXPORTPROJECTDATADIALOG_HPP
#define VADONEDITOR_UI_PROJECT_ASSET_EXPORTPROJECTDATADIALOG_HPP
#include <VadonEditor/UI/Project/Asset/ui_ExportProjectDataDialog.h>
#include <QDialog>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	// NOTE: this object is used as a "fire-and-forget" object by ExportProjectDataDialog
	// to message the asset server once it's initialized
	class ExportProjectDataBackend : public QObject
	{
		Q_OBJECT
	public:
		ExportProjectDataBackend(Core::Application& application, const QString& output_path);
		bool initialize();
	private slots:
		void received_message(const QByteArray& data);
	private:
		void cleanup();

		Core::Application& m_application;
		QString m_output_path;
	};

	class ExportProjectDataDialog : public QDialog
	{
		Q_OBJECT
	public:
		ExportProjectDataDialog(Core::Application& application, QWidget* parent = nullptr);
	protected:
		void accept() override;
	private slots:
		void browse_clicked();
	private:
		void update_controls();

		Core::Application& m_application;
		Ui::ExportProjectDataDialog m_ui;
	};
}
#endif