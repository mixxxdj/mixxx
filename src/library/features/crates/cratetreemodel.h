#ifndef CRATE_TREE_MODEL_H
#define CRATE_TREE_MODEL_H

#include <QModelIndex>
#include <QVariant>
#include <QList>
#include <QUrl>

#include "library/treeitemmodel.h"

// This class represents the crate tree for the nested crates feature

class CrateTreeModel : public TreeItemModel {
  public:
    CrateTreeModel(QObject *parent = 0);
    virtual ~CrateTreeModel();
};

#endif // CRATE_TREE_MODEL_H
