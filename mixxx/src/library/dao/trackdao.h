

#ifndef TRACKDAO_H
#define TRACKDAO_H

#include <QObject>
#include <QSqlDatabase>
#include "library/dao/cuedao.h"
#include "library/dao/dao.h"
class TrackInfoObject;

const QString LIBRARYTABLE_ID = "id";
const QString LIBRARYTABLE_ARTIST = "artist";
const QString LIBRARYTABLE_TITLE = "title";
const QString LIBRARYTABLE_ALBUM = "album";
const QString LIBRARYTABLE_YEAR = "year";
const QString LIBRARYTABLE_GENRE = "genre";
const QString LIBRARYTABLE_TRACKNUMBER = "tracknumber";
const QString LIBRARYTABLE_LOCATION = "location";
const QString LIBRARYTABLE_COMMENT = "comment";
const QString LIBRARYTABLE_DURATION = "duration";
const QString LIBRARYTABLE_BITRATE = "bitrate";
const QString LIBRARYTABLE_BPM = "bpm";
const QString LIBRARYTABLE_LENGTHINBYTES = "length_in_bytes";
const QString LIBRARYTABLE_CUEPOINT = "cuepoint";
const QString LIBRARYTABLE_URL = "url";
const QString LIBRARYTABLE_SAMPLERATE = "samplerate";
const QString LIBRARYTABLE_WAVESUMMARYHEX = "wavesummaryhex";
const QString LIBRARYTABLE_CHANNELS = "channels";
const QString LIBRARYTABLE_MIXXXDELETED = "mixxx_deleted";
const QString LIBRARYTABLE_DATETIMEADDED= "datetime_added";

class TrackDAO : public QObject { //// public DAO {
Q_OBJECT
  public:
    //TrackDAO() {};
    TrackDAO(QSqlDatabase& database, CueDAO& cueDao);
    virtual ~TrackDAO();
    void setDatabase(QSqlDatabase& database) { m_database = database; };
    
    void initialize();
    int getTrackId(QString location);
    QString getTrackLocation(int id);
    bool trackExistsInDatabase(QString location);
    void addTrack(QString location);
    void addTrack(TrackInfoObject * pTrack);
    void removeTrack(int id);
    TrackInfoObject *getTrackFromDB(QSqlQuery &query) const;
    TrackInfoObject *getTrack(int id) const;
    void markTrackLocationAsVerified(QString location);
    void invalidateTrackLocations(QString directory);
    void markUnverifiedTracksAsDeleted();
    void markTrackLocationsAsDeleted(QString directory);
  public slots:
    void updateTrackInDatabase(TrackInfoObject* pTrack);
  private:
    //Prevents evil copy constructors! (auto-generated ones by the compiler that don't compile)
    TrackDAO(TrackDAO&); 
    bool operator=(TrackDAO&);
    /***NOTE: If you get a compile error complaining about these, it means you're copying
              a track DAO, which is probably not what you meant to do. Did you declare:
                 TrackDAO m_trackDAO;
              instead of:
                 TrackDAO &m_trackDAO;
              Go back and check your code...
         -- Albert Nov 1/2009
     */          
    
    QSqlDatabase &m_database;
    CueDAO &m_cueDao;
    
};

#endif //TRACKDAO_H
