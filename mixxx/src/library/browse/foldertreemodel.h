#pragma once

#include <QModelIndex>
#include <QHash>

#include "library/treeitemmodel.h"

// This class represents a folder item within the Browse Feature
// The class is derived from TreeItemModel to support lazy model
// initialization.

class FolderTreeModel : public TreeItemModel {
    Q_OBJECT
  public:
    FolderTreeModel(QObject *parent = 0);
    virtual ~FolderTreeModel();
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    bool directoryHasChildren(const QString& path) const;
    void removeChildDirsFromCache(const QStringList& rootPaths);

  private:
    // Used for memorizing the results of directoryHasChildren.
    // Note: this means we won't see directory tree changes after the initial
    // tree population after first expansion. I.e. newly added directories won't
    // be displayed and removed dirs will remain in the sidebar tree.
    // removeChildDirsFromCache() can be used to reset selected directories.
    mutable QHash<QString, bool> m_directoryCache;
};
