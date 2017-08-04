#ifndef CRATE_TREE_MODEL_H
#define CRATE_TREE_MODEL_H

#include <QList>
#include <QMap>

#include "library/treeitemmodel.h"
#include "library/features/crates/cratemanager.h"
#include "library/crate/crate.h"

// This class represents the crate tree for the nested crates feature

class CrateTreeModel : public TreeItemModel {
  public:
    CrateTreeModel(LibraryFeature* pFeature,
                   CrateManager* pCrates);
    ~CrateTreeModel() override = default;

  public slots:
    void reloadTree() override;

  private:
    void populateTree(const QStringList& idPaths, QMap<CrateId,TreeItem*> treeCrates);
    void createRecursionEntry(TreeItem* pRootItem);

    LibraryFeature* m_pFeature;
    CrateManager* m_pCrates;

    parented_ptr<TreeItem> m_pRecursion;
};

#endif // CRATE_TREE_MODEL_H
