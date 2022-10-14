#pragma once

#include <QAbstractItemModel>
#include <QFileSystemWatcher>
#include <QFuture>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QQueue>
#include <QThreadPool>
#include <QVariant>
#include <functional>
#include <mutex>
#include <unordered_map>

#include "library/treeitem.h"
#include "library/treeitemmodel.h"
#include "util/mutex.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace std {
template<>
struct hash<QString> {
    std::size_t operator()(const QString& s) const noexcept {
        return (size_t)qHash(s);
    }
};
} // namespace std
#endif

class TreeItem;
// This class represents a folder item within the Browse Feature
// The class is derived from TreeItemModel to support lazy model
// initialization.

class FolderTreeModel : public TreeItemModel {
    Q_OBJECT

    using FolderQueue = QQueue<std::pair<QModelIndex, QString>>;
    // FIXME: this should be a Trie (prefix tree)
    using Cache = std::unordered_map<QString, bool>;

  public:
    using TreeItemList = QList<TreeItem*>*;

    FolderTreeModel(QObject* parent = 0);
    virtual ~FolderTreeModel();
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    void processFolder(const QModelIndex& parent, const QString& path) const;

  private:
    void directoryHasChildren(const QString& path);

  private slots:
    void dirModified(const QString& str);
    void addChildren(const QModelIndex& parent, TreeItemList children);
    void onHasSubDirectory(const QString& path);

  signals:
    void newChildren(const QModelIndex& parent, TreeItemList hildren);
    void hasSubDirectory(const QString& path);

  private:
    // Used for memoizing the results of directoryHasChildren
    Cache m_directoryCache;
    mutable MReadWriteLock m_cacheLock;
    QFileSystemWatcher m_fsWatcher;
    QThreadPool m_pool;
    mutable FolderQueue m_folderQueue;
    mutable std::mutex m_queueLock;
    std::atomic<bool> m_isRunning;
    QFuture<void> m_folderProcess;
};
Q_DECLARE_METATYPE(FolderTreeModel::TreeItemList);
Q_DECLARE_OPAQUE_POINTER(FolderTreeModel::TreeItemList);
