#include "library/cratehighlightdelegate.h"
#include "library/treeitem.h"
#include "trackinfoobject.h"
#include "libraryfeature.h"
#include "cratefeature.h"

CrateHighlightDelegate::CrateHighlightDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

// This will be called to by Qt before painting the TreeView-Item. Set up styles here
void CrateHighlightDelegate::initStyleOption(QStyleOptionViewItem* option,
                                             const QModelIndex& index) const {
    if (!index.isValid()) {
        return;
    }

    QStyledItemDelegate::initStyleOption(option, index);
    QStyleOptionViewItemV4 *optionV4 = qstyleoption_cast<QStyleOptionViewItemV4*>(option);

    // If the item has no parent then it is a top-level sidebar item and its
    // internalPointer is of type SidebarModel*, not TreeItem*.
    if (!index.parent().isValid()) {
        return;
    }

    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == NULL) {
        return;
    }

    LibraryFeature* pFeature = item->getFeature();
    if (pFeature == NULL) {
        return;
    }

    TrackPointer pTrack = pFeature->getSelectedTrack();
    if (pTrack.isNull()) {
        return;
    }

    if (pFeature->isTrackInChildModel(pTrack->getId(), item->dataPath())){
        optionV4->font.setBold(true);
    }
}
