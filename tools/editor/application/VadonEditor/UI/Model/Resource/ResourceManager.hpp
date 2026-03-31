#ifndef VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEMANAGER_HPP
#define VADONEDITOR_UI_MODEL_RESOURCE_RESOURCEMANAGER_HPP
#include <QObject>
#include <QHash>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
}
namespace VadonEditor::UI
{
	class ResourceManager : public QObject
	{
		Q_OBJECT
	public:
	private slots:
		void asset_opened(const QModelIndex& index);
		void resource_widget_removed(QObject* widget_obj);
	private:
		ResourceManager(Core::Application& application);

		bool initialize();
		void shutdown();

		Core::Application& m_application;
		QHash<QUuid, QWidget*> m_resource_widgets;
		QHash<QWidget*, QUuid> m_widget_reverse_lookup;

		friend class UISystem;
	};
}
#endif