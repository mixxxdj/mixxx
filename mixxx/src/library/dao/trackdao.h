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
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/dao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
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

class TrackDAO : public QObject, public virtual DAO {
    Q_OBJECT
  public:
    /** The 'config object' is necessary because users decide ID3 tags get
     * synchronized on track metadata change **/
    TrackDAO(QSqlDatabase& database, CueDAO& cueDao,
             PlaylistDAO& playlistDao, CrateDAO& crateDao,
             AnalysisDao& analysisDao,
             ConfigObject<ConfigValue>* pConfig = NULL);
    virtual ~TrackDAO();

    void finish();
    void setDatabase(QSqlDatabase& database) { m_database = database; };

    void initialize();
    int getTrackId(QString absoluteFilePath);
    bool trackExistsInDatabase(QString absoluteFilePath);
    QString getTrackLocation(int id);
    int addTrack(QString absoluteFilePath, bool unremove);
    int addTrack(QFileInfo& fileInfo, bool unremove);
    void addTracks(QList<TrackInfoObject*> tracksToAdd, bool unremove);
    QList<int> addTracks(QList<QFileInfo> fileInfoList, bool unremove);
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
    void detectMovedFiles();

  signals:
    void trackDirty(int trackId);
    void trackClean(int trackId);
    void trackChanged(int trackId);
    void tracksAdded(QSet<int> trackIds);
    void tracksRemoved(QSet<int> trackIds);

  public slots:
    // The public interface to the TrackDAO requires a TrackPointer so that we
    // have a guarantee that the track will not be deleted while we are working
    // on it. However, private parts of TrackDAO can use the raw saveTrack(TIO*)
    // call.
    void saveTrack(TrackPointer pTrack);

    // TrackDAO provides a cache of TrackInfoObject's that have been requested
    // via getTrack(). saveDirtyTracks() saves all cached tracks marked dirty
    // to the database.
    void saveDirtyTracks();

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
    TrackPointer getTrackFromDB(QSqlQuery &query) const;
    QString absoluteFilePath(QString location);

    void prepareTrackLocationsInsert(QSqlQuery& query);
    void bindTrackToTrackLocationsInsert(QSqlQuery& query, TrackInfoObject* pTrack);
    void prepareLibraryInsert(QSqlQuery& query);
    void bindTrackToLibraryInsert(QSqlQuery& query,
                                  TrackInfoObject* pTrack, int trackLocationId);

    void writeAudioMetaData(TrackInfoObject* pTrack);
    // Called when the TIO reference count drops to 0
    static void deleteTrack(TrackInfoObject* pTrack);

    QSqlDatabase &m_database;
    CueDAO &m_cueDao;
    PlaylistDAO &m_playlistDao;
    CrateDAO &m_crateDao;
    AnalysisDao& m_analysisDao;
    ConfigObject<ConfigValue> * m_pConfig;
    mutable QHash<int, TrackWeakPointer> m_tracks;
    mutable QSet<int> m_dirtyTracks;
    mutable QCache<int,TrackPointer> m_trackCache;

    DISALLOW_COPY_AND_ASSIGN(TrackDAO);
};

#endif //TRACKDAO_H
