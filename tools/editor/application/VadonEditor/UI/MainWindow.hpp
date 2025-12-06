#ifndef VADONEDITOR_UI_MAINWINDOW_HPP
#define VADONEDITOR_UI_MAINWINDOW_HPP
#include <QtWidgets/QMainWindow>

#include <VadonEditor/UI/Forms/ui_MainWindow.h>

namespace VadonEditor::UI
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT
    public:
        MainWindow(QWidget* parent = Q_NULLPTR);

        ~MainWindow();
    signals:
        void menu_test();
    public slots:
        void log_message(const QString& message);
        
        void generate_uuid_triggered();
    private:
        Ui::MainWindow m_ui;

        friend class Application;
    };
}
#endif