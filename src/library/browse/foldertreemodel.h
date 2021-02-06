#pragma once

#include <QAbstractItemModel>
#include <QFileSystemWatcher>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QThreadPool>

#include <unordered_map>

#include "library/treeitemmodel.h"
#include "library/treeitem.h"

class TreeItem;
// This class represents a folder item within the Browse Feature
// The class is derived from TreeItemModel to support lazy model
// initialization.

class FolderTreeModel : public TreeItemModel {
    Q_OBJECT
  public:
    FolderTreeModel(QObject *parent = 0);
    virtual ~FolderTreeModel();
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    void insertTreeItemRows(QList<TreeItem*> &rows, int position, const QModelIndex& parent = QModelIndex()) override;

  private:
    bool checkFS(const QString& path) const;

/*
  protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;
*/
  private slots:
    void directoryModified(const QString& str);
  private:
    // Used for memoizing the results of directoryHasChildren
    mutable std::unordered_map<QString, bool> m_directoryCache;
    mutable QFileSystemWatcher m_fsWatcher;
    QThreadPool m_pool;
};
