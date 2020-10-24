#include "library/coverartdelegate.h"

#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "util/assert.h"

CoverArtDelegate::CoverArtDelegate(
        QTableView* parent)
        : BaseCoverArtDelegate(parent) {
}

CoverInfo CoverArtDelegate::coverInfoForIndex(
        const QModelIndex& index) const {
    return m_pTrackModel->getCoverInfo(index);
}
