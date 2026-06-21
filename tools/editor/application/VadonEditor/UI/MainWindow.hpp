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

        QTabWidget* get_scene_tab_widget() const { return m_ui.sceneTabWidget; }
        RenderWidget* get_viewport() const { return m_ui.viewport; }
    signals:
        void run_simulator_requested();
        void stop_simulator_requested();
        void close_requested();
    private slots:
        void message_logged(QtMsgType type, const QString& message);

        void new_triggered();
        void quit_triggered();
        void generate_uuid_triggered();

        void project_settings_triggered();
        void run_plugin_triggered();
        void shutdown_plugin_triggered();

        void project_data_schema_triggered();
        void generate_data_schema_triggered();
    protected:
        void closeEvent(QCloseEvent* event) override;
    private:
        Core::Application& m_application;
        Ui::MainWindow m_ui;

        friend class UISystem;
    };
}
#endif