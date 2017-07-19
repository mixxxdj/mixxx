#include <QVector>

#include "library/trackcollection.h"
#include "library/features/crates/cratetreemodel.h"

#include "util/db/dbid.h"

namespace {

const CrateId rootId(-1);

} // anonymous namespace

CrateTreeModel::CrateTreeModel(LibraryFeature* pFeature,
                               TrackCollection* pTrackCollection)
    : m_pFeature(pFeature),
      m_pTrackCollection(pTrackCollection) {
}

void CrateTreeModel::fillTree(const QStringList& idPaths, QMap<CrateId,TreeItem*> treeCrates) {
    Crate parent, child;

    // looping through the paths we are gsonna get the parent crate and the child crate of
    // each path (2nd to last and last IDs respectivly). Then we insert the child under the
    // parent. Since it's sorted alphabetically the parent will always exist before the child
    for (const auto& idPath : idPaths) {
        // if there is no parent set the parent as the root of the tree
        if (!m_pTrackCollection->crates().findParentAndChildFromPath(parent, child, idPath)) {
            parent.setId(rootId);
        }
        if (child.getName() == "") {
            continue;
        }
        treeCrates.insert(child.getId(), // key is the id of child
                            // value is the treeItem returned by asdasdasppendChild()
                            treeCrates.value(parent.getId())->appendChild(child.getName(),
                                                                    child.getId().toInt()));

        //        treeCrates.value(child.getId())->setLabel(idPath);
    }
}

void CrateTreeModel::reloadTree() {
    const QStringList idPaths = m_pTrackCollection->crates().collectIdPaths();
    QMap<CrateId,TreeItem*> treeCrates;

    beginResetModel();
    // Create root item
    TreeItem* pRootItem = setRootItem(std::make_unique<TreeItem>(m_pFeature));
    treeCrates.insert(rootId, pRootItem);
    // Create recursion button
    m_pRecursion = parented_ptr<TreeItem>(pRootItem->appendChild(tr("Recursion"), ""));

    fillTree(idPaths, treeCrates);
    endResetModel();
}
