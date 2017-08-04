#ifndef MIXXX_CRATEMANAGER_H
#define MIXXX_CRATEMANAGER_H

#include <QSet>
#include <QList>

#include "library/crate/crate.h"
#include "track/trackid.h"
#include "util/db/sqlstorage.h"

#include "library/crate/cratesummary.h"
#include "library/features/crates/cratestoragehelpers.h"
#include "library/features/crates/cratestorage.h"
#include "library/features/crates/cratetracks.h"
#include "library/features/crates/cratehierarchy.h"

#include "util/db/fwdsqlqueryselectresult.h"
#include "util/db/sqlsubselectmode.h"

class CrateManager : public QObject, public virtual SqlStorage {
    Q_OBJECT

  public:
    CrateManager() = default;
    ~CrateManager() override = default;

    void repairDatabase(
      QSqlDatabase database) override;

    void connectDatabase(
            QSqlDatabase database) override;
    void disconnectDatabase() override;

    void checkClosure();

    ///////////////////
    // Member access //
    ///////////////////

    const inline CrateStorage& storage() const {
        return m_crateStorage;
    }

    const inline CrateTracks& tracks() const {
        return m_crateTracks;
    }

    const inline CrateHierarchy& hierarchy() const {
        return m_crateHierarchy;
    }

    inline bool isRecursionActive() const {
        return m_recursionStatus;
    }

    inline void setRecursionStatus(bool status) {
        m_recursionStatus = status;
    }

    /////////////////////////////////////////////
    // Wrappers for non const functions to be  //
    // called by trackCollection. They return  //
    // whether the transaction succeded or not //
    /////////////////////////////////////////////

    bool onPurgingTracks(const QList<TrackId>& trackIds);

    bool insertCrate(Crate& crate,
                     CrateId* pCrateId,
                     const Crate& parent = Crate());
    // update crate information (name, locked status, AutoDJ use)
    // in the database with info from Crate object
    bool updateCrate(const Crate& crate);
    bool deleteCrate(const Crate& crate);
    bool addTracksToCrate(CrateId crateId, const QList<TrackId>& trackIds);
    bool removeTracksFromCrate(CrateId crateId, const QList<TrackId>& trackIds);

    // called to update a crate's AutoDj status
    bool updateAutoDjCrate(CrateId crateId, bool isAutoDjSource);

  signals:
    void crateInserted(CrateId id);
    void crateUpdated(CrateId id);
    void crateDeleted(CrateId id);

    void crateTracksChanged(
            CrateId crate,
            const QList<TrackId>& tracksAdded,
            const QList<TrackId>& tracksRemoved);
    void crateSummaryChanged(
            const QSet<CrateId>& crates);

  private:
    void createViews();

    CrateStorage m_crateStorage; // Manages crate storage
    CrateTracks m_crateTracks; // Manages tracks on crates
    CrateHierarchy m_crateHierarchy; // Manages the hierarchy

    bool m_recursionStatus;
    QSqlDatabase m_database;
};

#endif // MIXXX_CRATEMANAGER_H
