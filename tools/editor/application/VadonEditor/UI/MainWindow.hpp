#ifndef VADONEDITOR_UI_MAINWINDOW_HPP
#define VADONEDITOR_UI_MAINWINDOW_HPP
#include <QtWidgets/QMainWindow>

#include <VadonEditor/UI/ui_MainWindow.h>
namespace VadonEditor::Core
{
    class Application;
}
namespace VadonEditor::UI
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        MainWindow(Core::Application& application, QWidget* parent = Q_NULLPTR);

        ~MainWindow();

        QWidget* get_viewport() const { return m_ui.viewport; }
    private slots:
        void message_logged(const QString& message);

        void new_triggered();
        void quit_triggered();
        void generate_uuid_triggered();
        void project_settings_triggered();
    private:
        Core::Application& m_application;
        Ui::MainWindow m_ui;

        friend class Application;
    };
}
#endif