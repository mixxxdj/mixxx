#ifndef TRACKCOLLECTION_H
#define TRACKCOLLECTION_H

#include <QtSql>
#include <QList>
#include <QSharedPointer>
#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/basetrackcache.h"
#include "library/crate/cratestorage.h"
#include "library/dao/trackdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"
#include "util/db/dbconnection.h"

// forward declaration(s)
class Track;

#define AUTODJ_TABLE "Auto DJ"

class BpmDetector;

// Manages everything around tracks.
class TrackCollection : public QObject {
    Q_OBJECT

  public:
    static const int kRequiredSchemaVersion;

    explicit TrackCollection(UserSettingsPointer pConfig);
    ~TrackCollection() override;

    bool checkForTables();

    void resetLibaryCancellation();

    QSqlDatabase& database() {
        return m_dbConnection.database();
    }

    const CrateStorage& crates() const {
        return m_crates;
    }

    TrackDAO& getTrackDAO();
    PlaylistDAO& getPlaylistDAO();
    DirectoryDAO& getDirectoryDAO();
    AnalysisDao& getAnalysisDAO() {
        return m_analysisDao;
    }
    QSharedPointer<BaseTrackCache> getTrackSource();
    void setTrackSource(QSharedPointer<BaseTrackCache> trackSource);
    void cancelLibraryScan();

    UserSettingsPointer getConfig() {
        return m_pConfig;
    }

    void relocateDirectory(QString oldDir, QString newDir);

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);

    bool purgeTracks(const QList<TrackId>& trackIds);
    bool purgeTracks(const QDir& dir);

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
    void crateSummaryChanged(
            const QSet<CrateId>& crates);

  private:
    UserSettingsPointer m_pConfig;
    DbConnection m_dbConnection;
    QSharedPointer<BaseTrackCache> m_defaultTrackSource;
    PlaylistDAO m_playlistDao;
    CrateStorage m_crates;
    CueDAO m_cueDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    LibraryHashDAO m_libraryHashDao;
    TrackDAO m_trackDao;
};

#endif // TRACKCOLLECTION_H
