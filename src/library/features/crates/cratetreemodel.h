#ifndef CRATE_TREE_MODEL_H
#define CRATE_TREE_MODEL_H

#include <QList>
#include <QMap>

#include "library/treeitemmodel.h"
#include "library/features/crates/cratehierarchy.h"
#include "library/crate/crate.h"

// This class represents the crate tree for the nested crates feature

class CrateTreeModel : public TreeItemModel {
  public:
    CrateTreeModel(LibraryFeature* pFeature,
                   TrackCollection* pTrackCollection);
          //virtual ~CrateTreeModel();

    // QVariant data(const QModelIndex& index, int role) const override;
    // bool setData(const QModelIndex& index, const QVariant& value, int role) override;

  public slots:
    void reloadTree() override;

  protected:

  private:
    void fillTree(const QStringList& idPaths);

    LibraryFeature* m_pFeature;
    TrackCollection* m_pTrackCollection;

    parented_ptr<TreeItem> m_pRecursion;
    QMap<CrateId,TreeItem*> m_treeCrates;

};

#endif // CRATE_TREE_MODEL_H
