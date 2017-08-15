#ifndef CRATE_TREE_MODEL_H
#define CRATE_TREE_MODEL_H

#include <QList>
#include <QMap>

#include "library/treeitemmodel.h"
#include "library/features/crates/cratemanager.h"
#include "library/crate/crate.h"

// This class represents the crate tree for the nested crates feature

//NOTE(gramanas): This model needs a way to update it's items without folding
// all the children. The TreeItemModel did not need to update on the fly
// since every feature that made use of it either didn't use hierarchy
// or did not need to add extra items once it was initialized.
// Atm the tree folds with every change in the crates, it's not optimal but
// it's usable.
class CrateTreeModel : public TreeItemModel {
  public:
    CrateTreeModel(LibraryFeature* pFeature,
                   CrateManager* pCrates);
    ~CrateTreeModel() override = default;

    static const QString RECURSION_DATA;

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
