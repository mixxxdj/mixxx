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
};
