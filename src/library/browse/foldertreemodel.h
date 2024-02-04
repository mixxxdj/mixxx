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

  private:
    // Used for memoizing the results of directoryHasChildren
    mutable QHash<QString, bool> m_directoryCache;
};
