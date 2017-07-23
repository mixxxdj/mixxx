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

    //////////////////////////////////////
    // wrappers for non const fucntions //
    // to be called by trackCollection  //
    //////////////////////////////////////

    bool onPurgingTracks(const QList<TrackId>& trackIds);

    ////////////////////////////////////////////////////////

    void crateCrate();
    void deleteCrate();
    void moveCrate();
    void renameCrate();
    void updateCrateTracks();
    void checkClosure() const;

    ////////////////////////////////////////////////////////

    bool insertCrate(const Crate& crate, CrateId* pCrateId = nullptr);
    bool updateCrate(const Crate& crate);
    bool deleteCrate(CrateId crateId);
    bool addCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);
    bool removeCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);

    bool updateAutoDjCrate(CrateId crateId, bool isAutoDjSource);

  signals:
    void crateInserted(CrateId id);
    void crateUpdated(CrateId id);
    void crateDeleted(CrateId id);

    void crateTracksChanged(
            CrateId crate,
            const QList<TrackId>& tracksAdded,
            const QList<TrackId>& tracksRemoved);
    //trackCollection @ unhideTracks()
    void crateSummaryChanged(
            const QSet<CrateId>& crates);

  private:
    void createViews();

    CrateStorage m_crateStorage; //Manages crate storage
    CrateTracks m_crateTracks; //Manages tracks on crates
    CrateHierarchy m_crateHierarchy; //Manages the hierarchy
    //CrateHierarchy should contain the data that the tree uses to display the crates
    //specifically the QMap<> and have wrapper functions to set it and get it
    QSqlDatabase m_database;

};

#endif // MIXXX_CRATEMANAGER_H
