#include "library/coverartdelegate.h"

#include "library/dao/trackschema.h"
#include "library/trackmodel.h"
#include "util/assert.h"

CoverArtDelegate::CoverArtDelegate(
        QTableView* parent)
        : BaseCoverArtDelegate(parent),
          m_iCoverSourceColumn(m_pTrackModel->fieldIndex(
                  LIBRARYTABLE_COVERART_SOURCE)),
          m_iCoverTypeColumn(m_pTrackModel->fieldIndex(
                  LIBRARYTABLE_COVERART_TYPE)),
          m_iCoverLocationColumn(m_pTrackModel->fieldIndex(
                  LIBRARYTABLE_COVERART_LOCATION)),
          m_iCoverHashColumn(m_pTrackModel->fieldIndex(
                  LIBRARYTABLE_COVERART_HASH)),
          m_iTrackIdColumn(m_pTrackModel->fieldIndex(
                  LIBRARYTABLE_ID)),
          m_iTrackLocationColumn(m_pTrackModel->fieldIndex(
                  TRACKLOCATIONSTABLE_LOCATION)) {
    DEBUG_ASSERT(m_iCoverSourceColumn >= 0);
    DEBUG_ASSERT(m_iCoverTypeColumn >= 0);
    DEBUG_ASSERT(m_iCoverLocationColumn >= 0);
    DEBUG_ASSERT(m_iCoverHashColumn >= 0);
    DEBUG_ASSERT(m_iTrackIdColumn >= 0);
    DEBUG_ASSERT(m_iTrackLocationColumn >= 0);
}

CoverInfo CoverArtDelegate::coverInfoForIndex(
        const QModelIndex& index) const {
    CoverInfo coverInfo;
    coverInfo = m_pTrackModel->getCoverInfo(index);
    return coverInfo;
}
