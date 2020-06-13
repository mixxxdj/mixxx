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
    coverInfo.hash = index.sibling(index.row(), m_iCoverHashColumn).data().toUInt();
    coverInfo.type = static_cast<CoverInfo::Type>(
            index.sibling(index.row(), m_iCoverTypeColumn).data().toInt());
    coverInfo.source = static_cast<CoverInfo::Source>(
            index.sibling(index.row(), m_iCoverSourceColumn).data().toInt());
    coverInfo.coverLocation =
            index.sibling(index.row(), m_iCoverLocationColumn).data().toString();
    coverInfo.trackLocation =
            index.sibling(index.row(), m_iTrackLocationColumn).data().toString();
    return coverInfo;
}
