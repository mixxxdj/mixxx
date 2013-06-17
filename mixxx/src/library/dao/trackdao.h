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

#include "configobject.h"
#include "library/dao/dao.h"
#include "trackinfoobject.h"
#include "util.h"

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_COMPOSER = "composer";
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
const QString LIBRARYTABLE_BPM_LOCK = "bpm_lock";

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

class TrackDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    // The 'config object' is necessary because users decide ID3 tags get
    // synchronized on track metadata change
    TrackDAO(QSqlDatabase& database, CueDAO& cueDao,
             PlaylistDAO& playlistDao, CrateDAO& crateDao,
             AnalysisDao& analysisDao,
             ConfigObject<ConfigValue>* pConfig);
    virtual ~TrackDAO();

    void finish();
    void setDatabase(QSqlDatabase& database) { m_database = database; };

    void initialize();
    int getTrackId(QString absoluteFilePath);
    QList<int> getTrackIds(QList<QFileInfo> files);
    bool trackExistsInDatabase(QString absoluteFilePath);
    QString getTrackLocation(int id);
    int addTrack(const QString& file, bool unremove);
    int addTrack(const QFileInfo& fileInfo, bool unremove);
    void addTracksPrepare();
    bool addTracksAdd(TrackInfoObject* pTrack, bool unremove);
    void addTracksFinish();
    QList<int> addTracks(const QList<QFileInfo> &fileInfoList, bool unremove);
    void hideTracks(QList<int> ids);
    void purgeTracks(QList<int> ids);
    void unhideTracks(QList<int> ids);
    TrackPointer getTrack(int id, bool cacheOnly=false) const;
    bool isDirty(int trackId);

    // Scanning related calls. Should be elsewhere or private somehow.
    void markTrackLocationAsVerified(QString location);
    void markTracksInDirectoriesAsVerified(QStringList directories);
    void invalidateTrackLocationsInLibrary(QString libraryPath);
    void markUnverifiedTracksAsDeleted();
    void markTrackLocationsAsDeleted(QString directory);
    void detectMovedFiles(QSet<int>* pTracksMovedSetNew, QSet<int>* pTracksMovedSetOld);
    void databaseTrackAdded(TrackPointer pTrack);
    void databaseTracksMoved(QSet<int> tracksMovedSetOld, QSet<int> tracksMovedSetNew);
    void verifyTracksOutside(const QString& libraryPath, volatile bool* pCancel);

  signals:
    void trackDirty(int trackId);
    void trackClean(int trackId);
    void trackChanged(int trackId);
    void tracksAdded(QSet<int> trackIds);
    void tracksRemoved(QSet<int> trackIds);
    void dbTrackAdded(TrackPointer pTrack);
    void progressVerifyTracksOutside(QString path);

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

  private slots:
    void slotTrackDirty(TrackInfoObject* pTrack);
    void slotTrackChanged(TrackInfoObject* pTrack);
    void slotTrackClean(TrackInfoObject* pTrack);
    void slotTrackSave(TrackInfoObject* pTrack);

  private:
    bool isTrackFormatSupported(TrackInfoObject* pTrack) const;
    void saveTrack(TrackInfoObject* pTrack);
    void updateTrack(TrackInfoObject* pTrack);
    void addTrack(TrackInfoObject* pTrack, bool unremove);
    TrackPointer getTrackFromDB(int id) const;
    QString absoluteFilePath(QString location);

    void bindTrackToTrackLocationsInsert(TrackInfoObject* pTrack);
    void bindTrackToLibraryInsert(TrackInfoObject* pTrack, int trackLocationId);

    void writeAudioMetaData(TrackInfoObject* pTrack);
    // Called when the TIO reference count drops to 0
    static void deleteTrack(TrackInfoObject* pTrack);

    QSqlDatabase &m_database;
    CueDAO &m_cueDao;
    PlaylistDAO &m_playlistDao;
    CrateDAO &m_crateDao;
    AnalysisDao& m_analysisDao;
    ConfigObject<ConfigValue> * m_pConfig;
    static QHash<int, TrackWeakPointer> m_sTracks;
    static QMutex m_sTracksMutex;
    mutable QCache<int,TrackPointer> m_trackCache;

    QSqlQuery* m_pQueryTrackLocationInsert;
    QSqlQuery* m_pQueryTrackLocationSelect;
    QSqlQuery* m_pQueryLibraryInsert;
    QSqlQuery* m_pQueryLibraryUpdate;
    QSqlQuery* m_pQueryLibrarySelect;
    ScopedTransaction* m_pTransaction;

    QSet<int> m_tracksAddedSet;

    DISALLOW_COPY_AND_ASSIGN(TrackDAO);
};

#endif //TRACKDAO_H
