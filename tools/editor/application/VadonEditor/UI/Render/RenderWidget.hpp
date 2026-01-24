#ifndef VADONEDITOR_UI_RENDER_RENDERWIDGET_HPP
#define VADONEDITOR_UI_RENDER_RENDERWIDGET_HPP
#include <QWidget>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class RenderWidget : public QWidget
	{
		Q_OBJECT
	public:
		~RenderWidget();
	protected:
		RenderWidget(Core::Application& application, int window_id, QWidget* parent = nullptr);

		void showEvent(QShowEvent* showEvent) override;

		void resizeEvent(QResizeEvent* resizeEvent) override;

		void paintEvent(QPaintEvent* paintEvent) override;

		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;

		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

		void wheelEvent(QWheelEvent* event) override;

		Core::Application& m_application;
		int m_window_id;

		friend class UISystem;
	};
}
#endif