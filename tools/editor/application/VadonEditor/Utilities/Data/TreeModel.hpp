#ifndef VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#define VADONEDITOR_UTILITIES_DATA_TREEMODEL_HPP
#include <QAbstractItemModel>
namespace VadonEditor::Utilities
{
    class TreeModelItem
    {
    public:
        explicit TreeModelItem(QVariantList data, TreeModelItem* parent_item = nullptr);

        bool insert_children(int position, int count, int columns);
        bool insert_columns(int position, int columns);

        bool remove_children(int position, int count);
        bool remove_columns(int position, int columns);

        TreeModelItem* child(int row);
        int child_count() const;

        int column_count() const;

        QVariant data(int column) const;
        bool set_data(int column, const QVariant& value);

        int row() const;

        TreeModelItem* parent_item();

        // TODO: use data roles!
        bool is_editable() const { return m_is_editable; }
        void set_editable(bool editable) { m_is_editable = editable; }
    private:
        std::vector<std::unique_ptr<TreeModelItem>> m_child_items;
        QVariantList m_item_data;
        TreeModelItem* m_parent_item;
        bool m_is_editable = false;
    };

    class TreeModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        Q_DISABLE_COPY_MOVE(TreeModel)

        explicit TreeModel(const QStringList& headers, QObject* parent = nullptr);
        ~TreeModel() override;

        // Read-only functions
        QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex{}) const override;
        QModelIndex parent(const QModelIndex& index) const override;

        int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
        int columnCount(const QModelIndex& parent = QModelIndex{}) const override;

        QVariant data(const QModelIndex& index, int role) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        // Editable functions
        Qt::ItemFlags flags(const QModelIndex& index) const override;

        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
        bool setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role = Qt::EditRole) override;

        bool insertColumns(int position, int columns, const QModelIndex& parent = QModelIndex{}) override;
        bool removeColumns(int position, int columns, const QModelIndex& parent = QModelIndex{}) override;

        bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex{}) override;
        bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex{}) override;

        TreeModelItem* get_item(const QModelIndex& index) const;
    private:
        std::unique_ptr<TreeModelItem> m_root_item;
    };
}
#endif