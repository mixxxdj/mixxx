#pragma once

#include "library/basecoverartdelegate.h"

class CoverArtDelegate final : public BaseCoverArtDelegate {
    Q_OBJECT

  public:
    explicit CoverArtDelegate(
            QTableView* parent);
    ~CoverArtDelegate() final = default;

  private:
    CoverInfo coverInfoForIndex(
            const QModelIndex& index) const final;

    int m_iCoverSourceColumn;
    int m_iCoverTypeColumn;
    int m_iCoverLocationColumn;
    int m_iCoverHashColumn;
    int m_iTrackIdColumn;
    int m_iTrackLocationColumn;
};
