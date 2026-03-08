#include <VadonEditor/Core/Project/TypeFilterModel.hpp>

#include <VadonEditor/Core/Application.hpp>

#include <VadonEditor/Core/Project/ProjectManager.hpp>

#include <VadonEditor/Utilities/UUID.hpp>

namespace VadonEditor::Core
{
    TypeFilterModel::TypeFilterModel(Application& application, QObject* parent)
        : QSortFilterProxyModel(parent)
        , m_application(application)
    {
    }

    void TypeFilterModel::set_root_type(const QUuid& type_uuid)
    {
        m_root_type = type_uuid;
        invalidateRowsFilter();
    }

    bool TypeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
    {
        if (m_root_type.isNull() == true)
        {
            return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
        }

        const QModelIndex row_index = sourceModel()->index(sourceRow, 0, sourceParent);
        const QUuid type_uuid = sourceModel()->data(row_index, static_cast<Qt::ItemDataRole>(TypeTreeDataRole::TYPE_UUID)).toUuid();
        
        if (m_root_type == type_uuid)
        {
            return true;
        }
        
        return m_application.get_project_manager().get_project_data_schema().is_base_of(Utilities::qt_uuid_to_vadon_uuid(m_root_type), Utilities::qt_uuid_to_vadon_uuid(type_uuid));
    }
}