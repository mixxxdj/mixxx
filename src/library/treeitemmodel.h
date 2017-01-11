#ifndef TREE_ITEM_MODEL_H
#define TREE_ITEM_MODEL_H

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QUrl>

#include "library/abstractmodelroles.h"
#include "library/treeitem.h"
#include "util/memory.h"

class LibraryFeature;

class TreeItemModel : public QAbstractItemModel {
    Q_OBJECT

  public:
    explicit TreeItemModel(QObject* parent = nullptr);
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

    using QAbstractItemModel::insertRows;
    virtual bool insertRows(QList<TreeItem*> &data, int position, int rows, const QModelIndex &parent = QModelIndex());

    TreeItem* setRootItem(std::unique_ptr<TreeItem> pRootItem);
    TreeItem* getRootItem() const {
        return m_pRootItem.get();
    }

    // Return the underlying TreeItem.
    // If the index is invalid, the root item is returned.
    TreeItem* getItem(const QModelIndex &index) const;

    bool dropAccept(const QModelIndex& index, QList<QUrl> urls, QObject* pSource);
    bool dragMoveAccept(const QModelIndex& index, QUrl url);

    static QString getBreadCrumbString(TreeItem* pTree);
    static QSize getDefaultIconSize();

  public slots:
    virtual void reloadTree();
    void triggerRepaint();

  protected:
    LibraryFeature* getFeatureFromIndex(const QModelIndex& index) const;

    std::unique_ptr<TreeItem> m_pRootItem;
};

#endif
