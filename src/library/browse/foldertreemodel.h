#pragma once

#include <QAbstractItemModel>
#include <QFileSystemWatcher>
#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QThreadPool>
#include <QQueue>

#include <unordered_map>

#include "library/treeitemmodel.h"
#include "library/treeitem.h"
#include "util/fileaccess.h"
#include "util/mutex.h"

class TreeItem;
// This class represents a folder item within the Browse Feature
// The class is derived from TreeItemModel to support lazy model
// initialization.

class FolderTreeModel : public TreeItemModel {
    Q_OBJECT

    using FolderQueue = QQueue<std::pair<QModelIndex, FileAccess&>>;
  public:
    using TreeItemList = QList<TreeItem*>*;

    FolderTreeModel(QObject *parent = 0);
    virtual ~FolderTreeModel();
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    bool directoryHasChildren(const QString& path) const;
    void processFolder(const QModelIndex& parent, const FileAccess& path);

  private slots:
    void dirModified(const QString& str);
    void addChildren(const QModelIndex& parent, TreeItemList children); 

  signals:
    void newChildren(const QModelIndex& parent, TreeItemList hildren); 

  private:
    // Used for memoizing the results of directoryHasChildren
    mutable std::unordered_map<QString, bool> m_directoryCache;
    mutable MReadWriteLock m_cacheLock;
    mutable QFileSystemWatcher m_fsWatcher;
    QThreadPool m_pool;
    FolderQueue m_folderQueue;
    std::atomic<bool> m_isRunning;
};
Q_DECLARE_METATYPE(FolderTreeModel::TreeItemList);
Q_DECLARE_OPAQUE_POINTER(FolderTreeModel::TreeItemList);
