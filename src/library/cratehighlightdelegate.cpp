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

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    LibraryFeature* pFeature = item->getFeature();
    if (pFeature) {
        TrackPointer pTrack = pFeature->getSelectedTrack();
        if (pTrack) {
            if (pFeature->isTrackInChildModel(pTrack->getId(), item->dataPath())){
                optionV4->font.setBold(true);
            }
        }
    }
}
