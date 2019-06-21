#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QList>
#include <QSqlDatabase>
#include <QString>

#include "preferences/usersettings.h"
#include "library/dao/dao.h"
#include "track/globaltrackcache.h"
#include "util/class.h"
#include "util/memory.h"

class SqlTransaction;
class PlaylistDAO;
class AnalysisDao;
class CueDAO;
class LibraryHashDAO;


class TrackDAO : public QObject, public virtual DAO, public virtual GlobalTrackCacheRelocator {
    Q_OBJECT
  public:
    // The 'config object' is necessary because users decide ID3 tags get
    // synchronized on track metadata change
    TrackDAO(
            CueDAO& cueDao,
            PlaylistDAO& playlistDao,
            AnalysisDao& analysisDao,
            LibraryHashDAO& libraryHashDao,
            UserSettingsPointer pConfig);
    ~TrackDAO() override;

    void initialize(const QSqlDatabase& database) override {
        m_database = database;
    }
    void finish();

    TrackId getTrackId(const QString& absoluteFilePath);
    QList<TrackId> getTrackIds(const QList<QFileInfo>& files);
    QList<TrackId> getTrackIds(const QDir& dir);

    // WARNING: Only call this from the main thread instance of TrackDAO.
    TrackPointer getTrack(TrackId trackId) const;

    // Returns a set of all track locations in the library.
    QSet<QString> getTrackLocations();
    QString getTrackLocation(TrackId trackId);

    TrackPointer addSingleTrack(const TrackFile& trackFile, bool unremove);
    QList<TrackId> addMultipleTracks(const QList<QFileInfo>& fileInfoList, bool unremove);

    void addTracksPrepare();
    TrackPointer addTracksAddFile(const TrackFile& trackFile, bool unremove);
    TrackId addTracksAddTrack(const TrackPointer& pTrack, bool unremove);
    void addTracksFinish(bool rollback = false);

    bool onHidingTracks(
            const QList<TrackId>& trackIds);
    void afterHidingTracks(
            const QList<TrackId>& trackIds);

    bool onUnhidingTracks(
            const QList<TrackId>& trackIds);
    void afterUnhidingTracks(
            const QList<TrackId>& trackIds);

    bool onPurgingTracks(
            const QList<TrackId>& trackIds);
    void afterPurgingTracks(
            const QList<TrackId>& trackIds);

    // Fetches trackLocation from the database or adds it. If searchForCoverArt
    // is true, searches the track and its directory for cover art via
    // asynchronous request to CoverArtCache. If adding or fetching the track
    // fails, returns a transient TrackPointer for trackLocation. If
    // pAlreadyInLibrary is non-NULL, sets it to whether trackLocation was
    // already in the database.
    TrackPointer getOrAddTrack(const QString& trackLocation,
                               bool processCoverArt,
                               bool* pAlreadyInLibrary);

    void markTracksAsMixxxDeleted(const QString& dir);

    // Scanning related calls. Should be elsewhere or private somehow.
    void markTrackLocationsAsVerified(const QStringList& locations);
    void markTracksInDirectoriesAsVerified(const QStringList& directories);
    void invalidateTrackLocationsInLibrary();
    void markUnverifiedTracksAsDeleted();
    bool detectMovedTracks(QSet<TrackId>* pTracksMovedSetOld,
                          QSet<TrackId>* pTracksMovedSetNew,
                          const QStringList& addedTracks,
                          volatile const bool* pCancel);

    bool verifyRemainingTracks(
            const QStringList& libraryRootDirs,
            volatile const bool* pCancel);

    void detectCoverArtForTracksWithoutCover(volatile const bool* pCancel,
                                        QSet<TrackId>* pTracksChanged);

    void saveTrack(Track* pTrack);

  signals:
    void trackDirty(TrackId trackId) const;
    void trackClean(TrackId trackId) const;
    void trackChanged(TrackId trackId);
    void tracksAdded(QSet<TrackId> trackIds);
    void tracksRemoved(QSet<TrackId> trackIds);
    void dbTrackAdded(TrackPointer pTrack);
    void progressVerifyTracksOutside(QString path);
    void progressCoverArt(QString file);
    void forceModelUpdate();

  public slots:
    void databaseTrackAdded(TrackPointer pTrack);
    void databaseTracksMoved(QSet<TrackId> tracksMovedSetOld, QSet<TrackId> tracksMovedSetNew);
    void databaseTracksChanged(QSet<TrackId> tracksChanged);

  private slots:
    void slotTrackDirty(Track* pTrack);
    void slotTrackChanged(Track* pTrack);
    void slotTrackClean(Track* pTrack);

  private:
    TrackPointer getTrackFromDB(TrackId trackId) const;

    bool updateTrack(Track* pTrack);

    // Callback for GlobalTrackCache
    TrackFile relocateCachedTrack(
            TrackId trackId,
            TrackFile fileInfo) override;

    QSqlDatabase m_database;

    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    AnalysisDao& m_analysisDao;
    LibraryHashDAO& m_libraryHashDao;

    UserSettingsPointer m_pConfig;

    std::unique_ptr<QSqlQuery> m_pQueryTrackLocationInsert;
    std::unique_ptr<QSqlQuery> m_pQueryTrackLocationSelect;
    std::unique_ptr<QSqlQuery> m_pQueryLibraryInsert;
    std::unique_ptr<QSqlQuery> m_pQueryLibraryUpdate;
    std::unique_ptr<QSqlQuery> m_pQueryLibrarySelect;
    std::unique_ptr<SqlTransaction> m_pTransaction;
    int m_trackLocationIdColumn;
    int m_queryLibraryIdColumn;
    int m_queryLibraryMixxxDeletedColumn;

    QSet<TrackId> m_tracksAddedSet;

    DISALLOW_COPY_AND_ASSIGN(TrackDAO);
};

#endif //TRACKDAO_H
