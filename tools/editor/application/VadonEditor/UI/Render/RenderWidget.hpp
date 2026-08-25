#ifndef VADONEDITOR_UI_RENDER_RENDERWIDGET_HPP
#define VADONEDITOR_UI_RENDER_RENDERWIDGET_HPP
#include <QWidget>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	struct RenderClientInfo
	{
		Core::Application* application = nullptr;
		int client_id = -1;

		bool is_valid() const { return application != nullptr; }
	};

	class RenderWidget : public QWidget
	{
		Q_OBJECT
	public:
		RenderWidget(QWidget* parent = nullptr);
		~RenderWidget();

		void register_client(const RenderClientInfo& client_info) { m_client_info = client_info; }
	protected:
		void showEvent(QShowEvent* showEvent) override;

		void resizeEvent(QResizeEvent* resizeEvent) override;

		void paintEvent(QPaintEvent* paintEvent) override;

		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;

		void mousePressEvent(QMouseEvent* event) override;
		void mouseReleaseEvent(QMouseEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;

		void wheelEvent(QWheelEvent* event) override;

		RenderClientInfo m_client_info;
	};
}
#endif