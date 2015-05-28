#include "library/backgroundcolordelegate.h"
#include "library/treeitem.h"
#include "trackinfoobject.h"
#include "libraryfeature.h"
#include "cratefeature.h"


void BackgroundColorDelegate::initStyleOption(QStyleOptionViewItem *option,
                     const QModelIndex &index) const{
    QStyledItemDelegate::initStyleOption(option, index);
    QStyleOptionViewItemV4 *optionV4 = qstyleoption_cast<QStyleOptionViewItemV4*>(option);

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    LibraryFeature* pFeature = static_cast<LibraryFeature*>(item->getFeature());
    if (pFeature){
        TrackPointer pTrack = pFeature->getSelectedTrack();
        if (pTrack){
            CrateFeature* pCrateFeature = static_cast<CrateFeature*>(pFeature);
            if (pCrateFeature){
                if (pCrateFeature->getCrateDao().isTrackInCrate(pTrack->getId(), item->dataPath().toInt())){
                    optionV4->font.setBold(true);
                    //Todo: now trigger repaint of the widget so that it will be applyed
                }
            }

        }
    }
}
