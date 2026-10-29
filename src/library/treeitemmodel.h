#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <memory>

class TreeItem;

class TreeItemModel : public QAbstractItemModel {
    Q_OBJECT
  public:
    static const int kDataRole = Qt::UserRole;
    static const int kBoldRole = Qt::UserRole + 1;

    explicit TreeItemModel(QObject *parent = 0);
    ~TreeItemModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    // Tell the compiler we don't mean to shadow insertRows.
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool setData(const QModelIndex &a_rIndex, const QVariant &a_rValue,
                         int a_iRole = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void insertTreeItemRows(
            std::vector<std::unique_ptr<TreeItem>>&&,
            int position,
            const QModelIndex& parent = QModelIndex());

    TreeItem* setRootItem(std::unique_ptr<TreeItem> pRootItem);
    TreeItem* getRootItem() const {
        return m_pRootItem.get();
    }
    /// Returns the QModelIndex of the Root element.
    const QModelIndex getRootIndex();

    // Return the underlying TreeItem.
    // If the index is invalid, the root item is returned.
    TreeItem* getItem(const QModelIndex &index) const;

    // Returns the first TreeItem whose payload (kDataRole) equals data,
    // searching the whole tree depth-first. Returns nullptr if not found.
    TreeItem* findItemByData(const QVariant& data) const;

    // Returns the QModelIndex of the given TreeItem, or an invalid
    // QModelIndex if the item is not part of the tree (e.g. the root item).
    // Items nested under a child node (e.g. playlists grouped under a year
    // node in the History feature) yield an index whose parent() resolves
    // back to that child node.
    QModelIndex indexFromItem(TreeItem* pTreeItem) const;

    void triggerRepaint();
    void triggerRepaint(const QModelIndex& index);

  private:
    // Depth-first search helper for findItemByData().
    TreeItem* findItemByDataRecursive(TreeItem* pTreeItem, const QVariant& data) const;

    std::unique_ptr<TreeItem> m_pRootItem;
};
