#pragma once

#include <QObject>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class QModelIndex;
typedef QList<QModelIndex> QModelIndexList;

// Provides the track list for the Fingerprint Queue panel.
// Standard library columns (Title, Artist, Duration …) come from the shared
// BaseTrackCache track source and need no special handling.  The single
// extra column — Status — is sourced from acoustid_queue and exposed as a
// virtual column in the SQL view.
class MusicBrainzQueueTableModel final : public BaseSqlTableModel {
    Q_OBJECT

  public:
    // Column indices for data that lives in the SQL view rather than in the
    // shared BaseTrackCache.  The SELECT in setTableModel() must list columns
    // in exactly this order so that m_rowInfo.columnValues[COL_*] lines up
    // with the right value.  Same mechanism as the played / preview columns
    // in BaseSqlTableModel.
    enum Columns {
        COL_ID = 0,     // library.id — always internal (hidden)
        COL_STATUS = 1, // acoustid_queue.status; NULL shown as "pending"
        COL_COUNT = 2   // sentinel: number of view-owned columns
    };

    MusicBrainzQueueTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~MusicBrainzQueueTableModel() final;

    void setTableModel();

    bool isColumnInternal(int column) final;
    Qt::ItemFlags flags(const QModelIndex& index) const final;
    Capabilities getCapabilities() const final;

    QString modelKey(bool noSearch) const override;

  protected:
    QVariant rawValue(const QModelIndex& index) const override;
};
