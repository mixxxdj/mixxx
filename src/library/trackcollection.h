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

// forward declaration(s)
class BaseTrackCache;

// Manages the internal database.
class TrackCollection : public QObject,
    public virtual /*implements*/ SqlStorage {
    Q_OBJECT

  public:
    TrackCollection(
            QObject* parent,
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

    void connectTrackSource(QSharedPointer<BaseTrackCache> pTrackSource);
    QWeakPointer<BaseTrackCache> disconnectTrackSource();

    QSharedPointer<BaseTrackCache> getTrackSource() const {
        return m_pTrackSource;
    }

    // This function returns a track ID of all file in the list not already visible,
    // it adds and unhides the tracks as well.
    QList<TrackId> resolveTrackIds(
            const QList<TrackFile> &trackFiles,
            TrackDAO::ResolveTrackIdFlags flags);
    QList<TrackId> resolveTrackIdsFromUrls(
            const QList<QUrl>& urls,
            bool addMissing);
    QList<TrackId> resolveTrackIdsFromLocations(
            const QList<QString>& locations);

    bool insertCrate(const Crate& crate, CrateId* pCrateId = nullptr);
    bool updateCrate(const Crate& crate);
    bool deleteCrate(CrateId crateId);
    bool addCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);
    bool removeCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);

    bool updateAutoDjCrate(CrateId crateId, bool isAutoDjSource);

    TrackPointer getTrackById(
            TrackId trackId) const;

    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const;
    TrackId getTrackIdByRef(
            const TrackRef& trackRef) const;

    // Only public for tests
    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr);
    TrackId addTrack(
            const TrackPointer& pTrack,
            bool unremove);

  signals:
    // Forwarded signals from LibraryScanner
    void scanTrackAdded(TrackPointer pTrack);

    // Forwarded signals from TrackDAO
    void trackClean(TrackId trackId);
    void trackDirty(TrackId trackId);
    void tracksAdded(QSet<TrackId> trackIds);
    void tracksChanged(QSet<TrackId> trackIds);
    void tracksRemoved(QSet<TrackId> trackIds);
    void multipleTracksChanged();

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
    friend class TrackCollectionManager;
    friend class Upgrade;

    // No parent during database schema upgrade
    TrackCollection(const UserSettingsPointer& pConfig)
            : TrackCollection(nullptr, pConfig) {
    }

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);
    void hideAllTracks(const QDir& rootDir);

    bool purgeTracks(const QList<TrackId>& trackIds);
    bool purgeAllTracks(const QDir& rootDir);

    bool addDirectory(const QString& dir);
    bool removeDirectory(const QString& dir);
    void relocateDirectory(QString oldDir, QString newDir);

    void saveTrack(Track* pTrack);

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
