#pragma once

#include <QDir>
#include <QList>
#include <QSharedPointer>
#include <QSqlDatabase>

#include "preferences/usersettings.h"
#include "library/crate/cratestorage.h"
#include "library/dao/trackdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"

class BaseTrackCache;

// Manages the internal database.
class TrackCollection : public QObject,
    public virtual /*implements*/ SqlStorage {
    Q_OBJECT

  public:
    explicit TrackCollection(
            const UserSettingsPointer& pConfig);
    ~TrackCollection() override;

    void repairDatabase(
            QSqlDatabase database) override;

    void connectDatabase(
            QSqlDatabase database) override;
    void disconnectDatabase() override;

    QSqlDatabase database() const {
        return m_database;
    }

    const CrateStorage& crates() const {
        return m_crates;
    }

    TrackDAO& getTrackDAO() {
        return m_trackDao;
    }
    PlaylistDAO& getPlaylistDAO() {
        return m_playlistDao;
    }
    DirectoryDAO& getDirectoryDAO() {
        return m_directoryDao;
    }
    AnalysisDao& getAnalysisDAO() {
        return m_analysisDao;
    }

    QSharedPointer<BaseTrackCache> getTrackSource() const {
        return m_pTrackSource;
    }
    void setTrackSource(QSharedPointer<BaseTrackCache> pTrackSource);

    void cancelLibraryScan();

    // This function returns a track ID of all file in the list not already visible,
    // it adds and unhides the tracks as well.
    QList<TrackId> resolveTrackIds(const QList<QFileInfo> &files,
            TrackDAO::ResolveTrackIdFlags flags);
    QList<TrackId> resolveTrackIdsFromUrls(const QList<QUrl>& urls,
            bool addMissing);
    QList<TrackId> resolveTrackIdsFromLocations(
            const QList<QString>& locations);

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);

    bool insertCrate(const Crate& crate, CrateId* pCrateId = nullptr);
    bool updateCrate(const Crate& crate);
    bool deleteCrate(CrateId crateId);
    bool addCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);
    bool removeCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);

    bool updateAutoDjCrate(CrateId crateId, bool isAutoDjSource);

    TrackPointer getTrackById(
            const TrackId& trackId) const;
    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr);

    // Might be called from any thread
    void exportTrackMetadata(Track* pTrack) const;

    // Must be called from the main thread
    void saveTrack(Track* pTrack);

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
    friend class Library;
    friend class Upgrade;

    void hideAllTracks(const QDir& rootDir);

    bool purgeTracks(const QList<TrackId>& trackIds);
    bool purgeAllTracks(const QDir& rootDir);

    bool addDirectory(const QString& dir);
    void relocateDirectory(QString oldDir, QString newDir);

    UserSettingsPointer m_pConfig;

    QSqlDatabase m_database;

    PlaylistDAO m_playlistDao;
    CrateStorage m_crates;
    CueDAO m_cueDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    LibraryHashDAO m_libraryHashDao;
    TrackDAO m_trackDao;

    QSharedPointer<BaseTrackCache> m_pTrackSource;
};
