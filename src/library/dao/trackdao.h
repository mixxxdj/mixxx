#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QFileInfo>
#include <QObject>
#include <QSet>
#include <QHash>
#include <QList>
#include <QSqlDatabase>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QCache>
#include <QString>

#include "configobject.h"
#include "library/dao/dao.h"
#include "trackinfoobject.h"
#include "util.h"

#define LIBRARY_TABLE "library"

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_ALBUMARTIST = "album_artist";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_COMPOSER = "composer";
const QString LIBRARYTABLE_GROUPING = "grouping";
const QString LIBRARYTABLE_TRACKNUMBER = "tracknumber";
const QString LIBRARYTABLE_FILETYPE = "filetype";
const QString LIBRARYTABLE_LOCATION = "location";
const QString LIBRARYTABLE_COMMENT = "comment";
const QString LIBRARYTABLE_DURATION = "duration";
const QString LIBRARYTABLE_BITRATE = "bitrate";
const QString LIBRARYTABLE_BPM = "bpm";
const QString LIBRARYTABLE_REPLAYGAIN = "replaygain";
const QString LIBRARYTABLE_CUEPOINT = "cuepoint";
const QString LIBRARYTABLE_URL = "url";
const QString LIBRARYTABLE_SAMPLERATE = "samplerate";
const QString LIBRARYTABLE_WAVESUMMARYHEX = "wavesummaryhex";
const QString LIBRARYTABLE_CHANNELS = "channels";
const QString LIBRARYTABLE_MIXXXDELETED = "mixxx_deleted";
const QString LIBRARYTABLE_DATETIMEADDED = "datetime_added";
const QString LIBRARYTABLE_HEADERPARSED = "header_parsed";
const QString LIBRARYTABLE_TIMESPLAYED = "timesplayed";
const QString LIBRARYTABLE_PLAYED = "played";
const QString LIBRARYTABLE_RATING = "rating";
const QString LIBRARYTABLE_KEY = "key";
const QString LIBRARYTABLE_KEY_ID = "key_id";
const QString LIBRARYTABLE_BPM_LOCK = "bpm_lock";
const QString LIBRARYTABLE_PREVIEW = "preview";
const QString LIBRARYTABLE_COVERART = "coverart";
const QString LIBRARYTABLE_COVERART_SOURCE = "coverart_source";
const QString LIBRARYTABLE_COVERART_TYPE = "coverart_type";
const QString LIBRARYTABLE_COVERART_LOCATION = "coverart_location";
const QString LIBRARYTABLE_COVERART_HASH = "coverart_hash";

const QString TRACKLOCATIONSTABLE_ID = "id";
const QString TRACKLOCATIONSTABLE_LOCATION = "location";
const QString TRACKLOCATIONSTABLE_FILENAME = "filename";
const QString TRACKLOCATIONSTABLE_DIRECTORY = "directory";
const QString TRACKLOCATIONSTABLE_FILESIZE = "filesize";
const QString TRACKLOCATIONSTABLE_FSDELETED = "fs_deleted";
const QString TRACKLOCATIONSTABLE_NEEDSVERIFICATION = "needs_verification";

class ScopedTransaction;
class PlaylistDAO;
class AnalysisDao;
class CueDAO;
class CrateDAO;
class LibraryHashDAO;

class TrackDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    // The 'config object' is necessary because users decide ID3 tags get
    // synchronized on track metadata change
    TrackDAO(QSqlDatabase& database, CueDAO& cueDao,
             PlaylistDAO& playlistDao, CrateDAO& crateDao,
             AnalysisDao& analysisDao, LibraryHashDAO& libraryHashDao,
             ConfigObject<ConfigValue>* pConfig = NULL);
    virtual ~TrackDAO();

    void finish();
    void setDatabase(QSqlDatabase& database) { m_database = database; }

    void initialize();
    int getTrackId(const QString& absoluteFilePath);
    QList<int> getTrackIds(const QList<QFileInfo>& files);
    bool trackExistsInDatabase(const QString& absoluteFilePath);
    // Returns a set of all track locations in the library.
    QSet<QString> getTrackLocations();
    QString getTrackLocation(const int id);
    int addTrack(const QString& file, bool unremove);
    int addTrack(const QFileInfo& fileInfo, bool unremove);
    void addTracksPrepare();
    bool addTracksAdd(TrackInfoObject* pTrack, bool unremove);
    void addTracksFinish(bool rollback=false);
    QList<int> addTracks(const QList<QFileInfo>& fileInfoList, bool unremove);
    void hideTracks(const QList<int>& ids);
    void purgeTracks(const QList<int>& ids);
    void purgeTracks(const QString& dir);
    void unhideTracks(const QList<int>& ids);
    TrackPointer getTrack(const int id, const bool cacheOnly=false) const;

    // Fetches trackLocation from the database or adds it. If searchForCoverArt
    // is true, searches the track and its directory for cover art via
    // asynchronous request to CoverArtCache. If adding or fetching the track
    // fails, returns a transient TrackPointer for trackLocation. If
    // pAlreadyInLibrary is non-NULL, sets it to whether trackLocation was
    // already in the database.
    TrackPointer getOrAddTrack(const QString& trackLocation,
                               bool processCoverArt,
                               bool* pAlreadyInLibrary);

    bool isDirty(int trackId);
    void markTracksAsMixxxDeleted(const QString& dir);

    // Scanning related calls. Should be elsewhere or private somehow.
    void markTrackLocationsAsVerified(const QStringList& locations);
    void markTracksInDirectoriesAsVerified(const QStringList& directories);
    void invalidateTrackLocationsInLibrary();
    void markUnverifiedTracksAsDeleted();
    void markTrackLocationsAsDeleted(const QString& directory);
    void detectMovedFiles(QSet<int>* tracksMovedSetNew, QSet<int>* tracksMovedSetOld);
    void verifyRemainingTracks();
    void detectCoverArtForUnknownTracks(volatile const bool* pCancel,
                                        QSet<int>* pTracksChanged);

  signals:
    void trackDirty(int trackId) const;
    void trackClean(int trackId);
    void trackChanged(int trackId);
    void tracksAdded(QSet<int> trackIds);
    void tracksRemoved(QSet<int> trackIds);
    void dbTrackAdded(TrackPointer pTrack);
    void progressVerifyTracksOutside(QString path);
    void progressCoverArt(QString file);

  public slots:
    // The public interface to the TrackDAO requires a TrackPointer so that we
    // have a guarantee that the track will not be deleted while we are working
    // on it. However, private parts of TrackDAO can use the raw saveTrack(TIO*)
    // call.
    void saveTrack(TrackPointer pTrack);

    // Clears the cached TrackInfoObjects, which can be useful when the
    // underlying database tables change (eg. during a library rescan,
    // we might detect that a track has been moved and modify the update
    // the tables directly.)
    void clearCache();

    void databaseTrackAdded(TrackPointer pTrack);
    void databaseTracksMoved(QSet<int> tracksMovedSetOld, QSet<int> tracksMovedSetNew);
    void databaseTracksChanged(QSet<int> tracksChanged);

  private slots:
    void slotTrackDirty(TrackInfoObject* pTrack);
    void slotTrackChanged(TrackInfoObject* pTrack);
    void slotTrackClean(TrackInfoObject* pTrack);
    // Called by ~TrackInfoObject right before the track is destroyed.
    void slotTrackDeleted(TrackInfoObject* pTrack);

  private:
    bool isTrackFormatSupported(TrackInfoObject* pTrack) const;
    void saveTrack(TrackInfoObject* pTrack);
    void updateTrack(TrackInfoObject* pTrack);
    void addTrack(TrackInfoObject* pTrack, bool unremove);
    TrackPointer getTrackFromDB(const int id) const;
    QString absoluteFilePath(QString location);

    void bindTrackToTrackLocationsInsert(TrackInfoObject* pTrack);
    void bindTrackToLibraryInsert(TrackInfoObject* pTrack, int trackLocationId);

    void writeAudioMetaData(TrackInfoObject* pTrack);

    QSqlDatabase& m_database;
    CueDAO& m_cueDao;
    PlaylistDAO& m_playlistDao;
    CrateDAO& m_crateDao;
    AnalysisDao& m_analysisDao;
    LibraryHashDAO& m_libraryHashDao;
    ConfigObject<ConfigValue>* m_pConfig;
    static QHash<int, TrackWeakPointer> m_sTracks;
    static QMutex m_sTracksMutex;
    mutable QCache<int, TrackPointer> m_trackCache;

    QSqlQuery* m_pQueryTrackLocationInsert;
    QSqlQuery* m_pQueryTrackLocationSelect;
    QSqlQuery* m_pQueryLibraryInsert;
    QSqlQuery* m_pQueryLibraryUpdate;
    QSqlQuery* m_pQueryLibrarySelect;
    ScopedTransaction* m_pTransaction;
    int m_trackLocationIdColumn;
    int m_queryLibraryIdColumn;
    int m_queryLibraryMixxxDeletedColumn;

    QSet<int> m_tracksAddedSet;

    DISALLOW_COPY_AND_ASSIGN(TrackDAO);
};

#endif //TRACKDAO_H
