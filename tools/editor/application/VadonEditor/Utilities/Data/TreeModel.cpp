#include <VadonEditor/Utilities/Data/TreeModel.hpp>

namespace VadonEditor::Utilities
{
	TreeModelItem::TreeModelItem(QVariantList data, TreeModelItem* parent_item)
		: m_item_data(std::move(data)), m_parent_item(parent_item)
	{
	}

	bool TreeModelItem::insert_children(int position, int count, int columns)
	{
		if ((position < 0) || (position > qsizetype(m_child_items.size())))
		{
			return false;
		}

		for (int row = 0; row < count; ++row) 
		{
			QVariantList data(columns);
			m_child_items.insert(m_child_items.cbegin() + position, std::make_unique<TreeModelItem>(data, this));
		}

		return true;
	}

	bool TreeModelItem::insert_columns(int position, int columns)
	{
		if ((position < 0) || (position > m_item_data.size()))
		{
			return false;
		}

		for (int column = 0; column < columns; ++column)
		{
			m_item_data.insert(position, QVariant());
		}

		for (auto& child : std::as_const(m_child_items))
		{
			child->insert_columns(position, columns);
		}

		return true;
	}

	bool TreeModelItem::remove_children(int position, int count)
	{
		if ((position < 0) || (position + count > qsizetype(m_child_items.size())))
		{
			return false;
		}

		for (int row = 0; row < count; ++row)
		{
			m_child_items.erase(m_child_items.cbegin() + position);
		}

		return true;
	}

	bool TreeModelItem::remove_columns(int position, int columns)
	{
		if ((position < 0) || (position + columns > m_item_data.size()))
		{
			return false;
		}

		for (int column = 0; column < columns; ++column)
		{
			m_item_data.remove(position);
		}

		for (auto& child : std::as_const(m_child_items))
		{
			child->remove_columns(position, columns);
		}

		return true;
	}

	TreeModelItem* TreeModelItem::child(int row)
	{
		return row >= 0 && row < child_count() ? m_child_items.at(row).get() : nullptr;
	}

	int TreeModelItem::child_count() const
	{
		return int(m_child_items.size());
	}

	int TreeModelItem::row() const
	{
		if (m_parent_item == nullptr)
		{
			return 0;
		}

		const auto it = std::find_if(m_parent_item->m_child_items.cbegin(), m_parent_item->m_child_items.cend(),
			[this](const std::unique_ptr<TreeModelItem>& TreeModelItem) 
			{
				return TreeModelItem.get() == this;
			}
		);

		if (it != m_parent_item->m_child_items.cend())
		{
			return std::distance(m_parent_item->m_child_items.cbegin(), it);
		}

		Q_ASSERT_X(false, "VadonEditor::Utilities::TreeModelItem::row", "Invalid tree state"); // should not happen
		return -1;
	}

	int TreeModelItem::column_count() const
	{
		return int(m_item_data.count());
	}

	QVariant TreeModelItem::data(int column) const
	{
		return m_item_data.value(column);
	}

	bool TreeModelItem::set_data(int column, const QVariant& value)
	{
		if ((column < 0) || (column >= m_item_data.size()))
		{
			return false;
		}

		m_item_data[column] = value;
		return true;
	}

	TreeModelItem* TreeModelItem::parent_item()
	{
		return m_parent_item;
	}

	TreeModel::TreeModel(const QStringList& headers, QObject* parent)
		: QAbstractItemModel(parent)
	{
		QVariantList root_data;
		for (const QString& header : headers)
		{
			root_data << header;
		}

		m_root_item = std::make_unique<TreeModelItem>(root_data);
	}

	TreeModel::~TreeModel() = default;

	QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
	{
		if (hasIndex(row, column, parent) == false)
		{
			return QModelIndex{};
		}

		TreeModelItem* parent_item = parent.isValid()
			? static_cast<TreeModelItem*>(parent.internalPointer())
			: m_root_item.get();

		if (auto* child_item = parent_item->child(row))
		{
			return createIndex(row, column, child_item);
		}

		return QModelIndex{};
	}

	QModelIndex TreeModel::parent(const QModelIndex& index) const
	{
		if (!index.isValid())
		{
			return {};
		}

		auto* child_item = static_cast<TreeModelItem*>(index.internalPointer());
		TreeModelItem* parent_item = child_item->parent_item();

		return parent_item != m_root_item.get() ? createIndex(parent_item->row(), 0, parent_item) : QModelIndex{};
	}

	int TreeModel::rowCount(const QModelIndex& parent) const
	{
		if (parent.column() > 0)
		{
			return 0;
		}

		const TreeModelItem* parent_item = parent.isValid()
			? static_cast<const TreeModelItem*>(parent.internalPointer())
			: m_root_item.get();

		return parent_item->child_count();
	}

	int TreeModel::columnCount(const QModelIndex& parent) const
	{
		if (parent.isValid())
		{
			return static_cast<TreeModelItem*>(parent.internalPointer())->column_count();
		}

		return m_root_item->column_count();
	}

	QVariant TreeModel::data(const QModelIndex& index, int role) const
	{
		if (index.isValid() == false)
		{
			return QVariant{};
		}

		switch (role)
		{
		case Qt::DisplayRole:
		{
			const auto* item = static_cast<const TreeModelItem*>(index.internalPointer());
			return item->data(index.column());
		}
		// TODO: other roles!
		default:
			return QVariant{};
		}
	}

	QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
	{
		if (orientation != Qt::Horizontal)
		{
			return QVariant{};
		}

		switch (role)
		{
		case Qt::DisplayRole:
		{
			return m_root_item->data(section);
		}
		// TODO: other roles!
		default:
			return QVariant{};
		}
	}

	Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
	{
		if (index.isValid() == false)
		{
			return Qt::NoItemFlags;
		}

		Qt::ItemFlags flags = get_item(index)->is_editable() ? Qt::ItemFlag::ItemIsEditable : Qt::ItemFlag::NoItemFlags;
		return flags | QAbstractItemModel::flags(index);
	}

	bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
	{
		// TODO: other roles?
		if (role != Qt::EditRole)
		{
			return false;
		}

		TreeModelItem* item = get_item(index);
		bool result = item->set_data(index.column(), value);

		if (result)
		{
			emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
		}

		return result;
	}

	bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
		const QVariant& value, int role)
	{
		// TODO: other roles?
		if ((role != Qt::EditRole) || (orientation != Qt::Horizontal))
		{
			return false;
		}

		const bool result = m_root_item->set_data(section, value);

		if (result)
		{
			emit headerDataChanged(orientation, section, section);
		}

		return result;
	}

	bool TreeModel::insertColumns(int position, int columns, const QModelIndex& parent)
	{
		// NOTE: columns are inserted for all items!
		beginInsertColumns(parent, position, position + columns - 1);
		const bool success = m_root_item->insert_columns(position, columns);
		endInsertColumns();

		return success;
	}
	
	bool TreeModel::removeColumns(int position, int columns, const QModelIndex& parent)
	{
		// NOTE: columns are removed for all items!
		beginRemoveColumns(parent, position, position + columns - 1);
		const bool success = m_root_item->remove_columns(position, columns);
		endRemoveColumns();

		if (m_root_item->column_count() == 0)
		{
			removeRows(0, rowCount());
		}

		return success;
	}

	bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent)
	{
		TreeModelItem* parent_item = get_item(parent);
		if (parent_item == nullptr)
		{
			return false;
		}

		beginInsertRows(parent, position, position + rows - 1);
		const bool success = parent_item->insert_children(position, rows, m_root_item->column_count());
		endInsertRows();

		return success;
	}

	bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent)
	{
		TreeModelItem* parent_item = get_item(parent);
		if (parent_item == nullptr)
		{
			return false;
		}

		beginRemoveRows(parent, position, position + rows - 1);
		const bool success = parent_item->remove_children(position, rows);
		endRemoveRows();

		return success;
	}

	TreeModelItem* TreeModel::get_item(const QModelIndex& index) const
	{
		if (index.isValid() == true)
		{
			if (auto* item = static_cast<TreeModelItem*>(index.internalPointer()))
			{
				return item;
			}
		}
		return m_root_item.get();
	}
}