#pragma once

#include <gtest/gtest_prod.h>

#include <QList>
#include <QSharedPointer>
#include <QSqlDatabase>

#include "library/dao/analysisdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/directorydao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackdao.h"
#include "library/trackset/crate/cratestorage.h"
// Eve
#include "library/trackset/searchcrate/searchcratestorage.h"
// Eve
#include "preferences/usersettings.h"
#include "util/thread_affinity.h"

// forward declaration(s)
class BaseTrackCache;
class QDir;

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
            const QSqlDatabase& database) override;

    void connectDatabase(
            const QSqlDatabase& database) override;
    void disconnectDatabase() override;

    QSqlDatabase database() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_database;
    }

    QList<mixxx::FileInfo> loadRootDirs(
            bool skipInvalidOrMissing = false) const;
    QStringList getRootDirStrings() const;

    const CrateStorage& crates() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_crates;
    }

    const SearchCrateStorage& searchCrates() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_searchCrates;
    }

    TrackDAO& getTrackDAO() {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_trackDao;
    }
    PlaylistDAO& getPlaylistDAO() {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_playlistDao;
    }
    const DirectoryDAO& getDirectoryDAO() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_directoryDao;
    }
    AnalysisDao& getAnalysisDAO() {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_analysisDao;
    }

    void connectTrackSource(QSharedPointer<BaseTrackCache> pTrackSource);
    QWeakPointer<BaseTrackCache> disconnectTrackSource();

    QSharedPointer<BaseTrackCache> getTrackSource() const {
        DEBUG_ASSERT_QOBJECT_THREAD_AFFINITY(this);
        return m_pTrackSource;
    }

    bool insertCrate(const Crate& crate, CrateId* pCrateId = nullptr);
    bool updateCrate(const Crate& crate);
    bool deleteCrate(CrateId crateId);
    bool addCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);
    bool removeCrateTracks(CrateId crateId, const QList<TrackId>& trackIds);

    bool insertSearchCrate(const SearchCrate& searchCrate, SearchCrateId* pSearchCrateId = nullptr);
    bool updateSearchCrate(const SearchCrate& searchCrate);
    bool deleteSearchCrate(SearchCrateId searchCrateId);
    bool addSearchCrateTracks(SearchCrateId searchCrateId, const QList<TrackId>& trackIds);
    //    bool removeSearchCrateTracks(SearchCrateId searchCrateId, const QList<TrackId>& trackIds);
    // EVE

    bool updateAutoDjCrate(CrateId crateId, bool isAutoDjSource);

    TrackId getTrackIdByRef(
            const TrackRef& trackRef) const;

  signals:
    // Forwarded signals from LibraryScanner
    void scanTrackAdded(TrackPointer pTrack);

    // Forwarded signals from TrackDAO
    void trackClean(TrackId trackId);
    void trackDirty(TrackId trackId);
    void tracksAdded(const QSet<TrackId>& trackIds);
    void tracksChanged(const QSet<TrackId>& trackIds);
    void tracksRemoved(const QSet<TrackId>& trackIds);
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

    // Eve
    void searchCrateInserted(SearchCrateId id);
    void searchCrateUpdated(SearchCrateId id);
    void searchCrateDeleted(SearchCrateId id);

    void searchCrateTracksChanged(
            SearchCrateId searchCrate,
            const QList<TrackId>& tracksAdded,
            const QList<TrackId>& tracksRemoved);
    void searchCrateSummaryChanged(
            const QSet<SearchCrateId>& searchCrate);
    // Eve

  private:
    friend class TrackCollectionManager;
    friend class Upgrade;

    // No parent during database schema upgrade
    explicit TrackCollection(const UserSettingsPointer& pConfig)
            : TrackCollection(nullptr, pConfig) {
    }

    // TODO: All functions that load tracks or that may add tracks
    // will soon require additional context data that is provided
    // by TrackCollectionManager as an additional parameter. These
    // functions must only be invoked by TrackCollectionManager and
    // therefore don't appear in the public interface of this class.
    // See also: https://github.com/mixxxdj/mixxx/pull/2656

    // This function returns a track ID of all file in the list not already visible,
    // it adds and unhides the tracks as well.
    QList<TrackId> resolveTrackIds(
            const QList<mixxx::FileInfo>& trackFiles,
            TrackDAO::ResolveTrackIdFlags flags);
    QList<TrackId> resolveTrackIds(
            const QList<QUrl>& urls,
            TrackDAO::ResolveTrackIdFlags flags);
    QList<TrackId> resolveTrackIdsFromUrls(
            const QList<QUrl>& urls,
            bool addMissing);
    QList<TrackId> resolveTrackIdsFromLocations(
            const QList<QString>& locations);

    TrackPointer getTrackById(
            TrackId trackId) const;
    TrackPointer getTrackByRef(
            const TrackRef& trackRef) const;

    TrackPointer getOrAddTrack(
            const TrackRef& trackRef,
            bool* pAlreadyInLibrary = nullptr);
    FRIEND_TEST(DirectoryDAOTest, relocateDirectory);
    FRIEND_TEST(TrackDAOTest, detectMovedTracks);
    TrackId addTrack(
            const TrackPointer& pTrack,
            bool unremove);

    bool hideTracks(const QList<TrackId>& trackIds);
    bool unhideTracks(const QList<TrackId>& trackIds);
    void hideAllTracks(const QDir& rootDir);

    bool purgeTracks(const QList<TrackId>& trackIds);
    bool purgeAllTracks(const QDir& rootDir);

    DirectoryDAO::AddResult addDirectory(const mixxx::FileInfo& rootDir);
    DirectoryDAO::RemoveResult removeDirectory(const mixxx::FileInfo& rootDir);
    DirectoryDAO::RelocateResult relocateDirectory(const QString& oldDir, const QString& newDir);

    bool saveTrack(Track* pTrack) const;

    QSqlDatabase m_database;

    PlaylistDAO m_playlistDao;
    CrateStorage m_crates;
    // Eve
    SearchCrateStorage m_searchCrates;
    // Eve
    CueDAO m_cueDao;
    DirectoryDAO m_directoryDao;
    AnalysisDao m_analysisDao;
    LibraryHashDAO m_libraryHashDao;
    TrackDAO m_trackDao;

    QSharedPointer<BaseTrackCache> m_pTrackSource;
};
