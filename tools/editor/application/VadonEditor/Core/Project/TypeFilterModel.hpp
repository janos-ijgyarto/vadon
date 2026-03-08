#ifndef VADONEDITOR_CORE_PROJECT_TYPEFILTERMODEL_HPP
#define VADONEDITOR_CORE_PROJECT_TYPEFILTERMODEL_HPP
#include <QSortFilterProxyModel>
#include <QUuid>
namespace VadonEditor::Core
{
	class Application;
	class TypeFilterModel : public QSortFilterProxyModel
	{
		Q_OBJECT
	public:
		TypeFilterModel(Application& application, QObject* parent = nullptr);

        void set_root_type(const QUuid& type_uuid);

        bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
    protected:
		Application& m_application;
        QUuid m_root_type;
	};
}
#endif