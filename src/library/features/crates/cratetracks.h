#ifndef MIXXX_CRATETRACKS_H
#define MIXXX_CRATETRACKS_H

#include <QObject>
#include <QList>
#include <QSet>

#include "library/dao/dao.h"
#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestoragehelpers.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"

class CrateTracks : public virtual DAO {
  public:
    CrateTracks() {}
    ~CrateTracks() override {}

    void initialize(const QSqlDatabase& database) override;

    /////////////////////////////////////////////////////////////////////////
    // Crate write operations (transactional, non-const)
    /////////////////////////////////////////////////////////////////////////

    bool onAddingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onRemovingCrateTracks(
            CrateId crateId,
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);

    /////////////////////////////////////////////////////////////////////////
    // Const methods
    /////////////////////////////////////////////////////////////////////////

    // Crate content, i.e. the crate's tracks referenced by id
    uint countCrateTracks(CrateId crateId) const;

    // Format a subselect query for the tracks contained in crate.
    static QString formatSubselectQueryForCrateTrackIds(
            CrateId crateId); // no db access

    // Select the track ids of a crate or the crate ids of a track respectively.
    // The results are sorted (ascending) by the target id, i.e. the id that is
    // not provided for filtering. This enables the caller to perform efficient
    // binary searches on the result set after storing it in a list or vector.
    CrateTrackSelectResult selectCrateTracksSorted(
            CrateId crateId) const;
    CrateTrackSelectResult selectTrackCratesSorted(
            TrackId trackId) const;
    CrateTrackSelectResult selectTracksSortedByCrateNameLike(
            const QString& crateNameLike) const;

    // Returns the set of crate ids for crates that contain any of the
    // provided track ids.
    QSet<CrateId> collectCrateIdsOfTracks(
            const QList<TrackId>& trackIds) const;

  private:
    QSqlDatabase m_database;
};


#endif // MIXXX_CRATETRACKS_H
