
#include <QtDebug>
#include <QtCore>
#include <QSqlQuery>
#include <QSqlError>
#include "trackinfoobject.h"
#include "library/dao/trackdao.h"

TrackDAO::TrackDAO(QSqlDatabase& database, CueDAO& cueDao)
        : m_database(database),
          m_cueDao(cueDao) {

}

TrackDAO::~TrackDAO()
{
}

void TrackDAO::initialize()
{
    //Start the transaction
    m_database.transaction();
    
 	QSqlQuery query(m_database);
    if (!query.exec("CREATE TABLE IF NOT EXISTS track_locations (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "location varchar(512) UNIQUE, "
                    "filename varchar(512), "
                    "directory varchar(512), "
                    "fs_deleted INTEGER, "
                    "needs_verification INTEGER)"))
    {
        qDebug() << "Invalid track_locations schema, d'oh!" << __FILE__ << __LINE__;
        exit(-1);
    }
    //query.finish();

 	//Little bobby tables
    if (!query.exec("CREATE TABLE IF NOT EXISTS library (id INTEGER primary key AUTOINCREMENT, "
               "artist varchar(48), title varchar(48), "
               "album varchar(48), year varchar(16), "
               "genre varchar(32), tracknumber varchar(3), "
               "location varchar(512) REFERENCES track_locations(location), "
               "comment varchar(20), url varchar(256), "
               "duration integer, length_in_bytes integer, "
               "bitrate integer, samplerate integer, "
               "cuepoint integer, bpm float, "
               "wavesummaryhex blob, "
               "channels integer, "
               "datetime_added DEFAULT CURRENT_TIMESTAMP, "
               "mixxx_deleted integer)"))
    {
        qDebug() << "Invalid library schema, d'oh!" << __FILE__ << __LINE__;
        exit(-1);
    }
    //query.finish();
    
    m_database.commit();
}

/** Retrieve the track id for the track that's located at "location" on disk.
    @return the track id for the track located at location, or -1 if the track
            is not in the database.
*/
int TrackDAO::getTrackId(QString location)
{
    //Start the transaction
    m_database.transaction();

    QSqlQuery query(m_database);
    //Get the id of the track location, so we can get the Library table's track entry.
    int trackLocationId = -1;
    query.prepare("SELECT * FROM track_locations WHERE location=:location");
    query.bindValue(":location", location);
    query.exec();
    while (query.next()) {
        trackLocationId = query.value(query.record().indexOf("id")).toInt();
    }
    //query.finish();
    //Q_ASSERT(trackLocationId >= 0);
        
    int libraryTrackId = -1;
    query.exec("SELECT * FROM Library WHERE location=" + QString("%1").arg(trackLocationId));
    while (query.next()) {
        libraryTrackId = query.value(query.record().indexOf("id")).toInt();
    }
    //Q_ASSERT(libraryTrackId >= 0);
    //query.finish();

    m_database.commit();

    return libraryTrackId;
}

/** Some code (eg. drag and drop) needs to just get a track's location, and it's
    not worth retrieving a whole TrackInfoObject.*/
QString TrackDAO::getTrackLocation(int trackId)
{
    QSqlQuery query(m_database);
    int trackLocationId = -1;
    query.exec("SELECT * FROM Library WHERE id=" + QString("%1").arg(trackId));
    while (query.next()) {
        trackLocationId = query.value(query.record().indexOf("location")).toInt();
    }
    //query.finish();
    
    QString trackLocation = "";
    query.prepare("SELECT * FROM track_locations WHERE id=:id");
    query.bindValue(":id", trackLocationId);
    query.exec();
    while (query.next()) {
        trackLocation = query.value(query.record().indexOf("location")).toString();
    }
    //query.finish();
    
    return trackLocation;
}

/** Check if a track exists in the database already.
    @param file_location The full path to the track on disk, including the filename.
    @return true if the track is found in the database, false otherwise.
*/
bool TrackDAO::trackExistsInDatabase(QString location)
{
    return (getTrackId(location) != -1);
}


void TrackDAO::addTrack(QString location)
{
    QFileInfo file(location);
    TrackInfoObject * pTrack = new TrackInfoObject(file.absoluteFilePath());
    if (pTrack) {
        //Add the song to the database.
        this->addTrack(pTrack);
        delete pTrack;
    }
}

void TrackDAO::addTrack(TrackInfoObject * pTrack)
{

 	//qDebug() << "TrackCollection::addTrack(), inserting into DB";
    Q_ASSERT(pTrack); //Why you be giving me NULL pTracks

    //Start the transaction
    m_database.transaction();

 	QSqlQuery query(m_database);
 	int trackLocationId = -1;

    //Insert the track location into the corresponding table. This will fail silently
    //if the location is already in the table because it has a UNIQUE constraint.
    query.prepare("INSERT INTO track_locations (location, directory, filename, fs_deleted, needs_verification) "
                  "VALUES (:location, :directory, :filename, :fs_deleted, :needs_verification)");
    query.bindValue(":location", pTrack->getLocation());
    query.bindValue(":directory", QFileInfo(pTrack->getLocation()).path());
    query.bindValue(":filename", pTrack->getFilename());
    query.bindValue(":fs_deleted", 0);
    query.bindValue(":needs_verification", 0);
    query.exec();

 	//Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
    //query.finish();

    //Get the id of the track location, so we can make the Library table reference it.
    query.prepare("SELECT id FROM track_locations WHERE location=:location");
    query.bindValue(":location", pTrack->getLocation());
    query.exec();
    while (query.next()) {
        trackLocationId = query.value(query.record().indexOf("id")).toInt();
    }

 	//Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
    //query.finish();

    //Failure of this assert indicates that we were unable to insert the track location
    //into the table AND we could not retrieve the id of that track location from
    //the same table. "It shouldn't happen"... unless I screwed up - Albert :)
    Q_ASSERT(trackLocationId >= 0);

    query.prepare("INSERT INTO library (artist, title, album, year, genre, tracknumber, "
                  "location, comment, url, duration, length_in_bytes, "
                  "bitrate, samplerate, cuepoint, bpm, wavesummaryhex, "
                  "channels, mixxx_deleted) "
                  "VALUES (:artist, "
                  ":title, :album, :year, :genre, :tracknumber, "
                  ":location, :comment, :url, :duration, :length_in_bytes, "
                  ":bitrate, :samplerate, :cuepoint, :bpm, :wavesummaryhex, "
                  ":channels, :mixxx_deleted)");
    //query.bindValue(":id", 1001);
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":location", trackLocationId);
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":length_in_bytes", pTrack->getLength());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    QByteArray* pWaveSummary = pTrack->getWaveSummary();
    if (pWaveSummary) //Avoid null pointer deref
        query.bindValue(":wavesummaryhex", *pWaveSummary);
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    //query.bindValue(":datetime_added", pTrack->getDateAdded());
    query.bindValue(":channels", pTrack->getChannels());
    query.bindValue(":mixxx_deleted", 0);

    if (!query.exec())
    {
        qDebug() << "Failed to INSERT new track into library" << __FILE__ << __LINE__ << query.lastError();
        return;
    }
    //query.finish();

    query.prepare("SELECT id from library WHERE location = :location_id");
    query.bindValue(":location_id", trackLocationId);

    if (query.exec()) {
        if (query.next()) {
            int trackId = query.value(0).toInt();
            m_cueDao.saveTrackCues(trackId, pTrack);
        }
    } else {
        qDebug() << "Could not get track ID to save the track cue points:"
                 << query.lastError();
    }
    //query.finish();

 	//If addTrack() is called on a track that already exists in the library but has been "removed"
 	//(ie. mixxx_deleted is 1), then the above INSERT will fail silently. What we really want to
 	//do is just mark the track as undeleted, by setting mixxx_deleted to 0.
 	//addTrack() will not get called on files that are already in the library during a rescan
 	//(even if mixxx_deleted=1). However, this function WILL get called when a track is
 	//dragged and dropped onto the library or when manually imported from the File... menu.
 	//This allows people to re-add tracks that they "removed"...
    query.prepare("UPDATE library "
                  "SET mixxx_deleted=0 "
                  "WHERE location==" + QString("%1").arg(trackLocationId));
    query.exec();
    //query.finish();

    //Commit the transaction
    m_database.commit();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
     	qDebug() << query.lastError();
    }
}

  /** Removes a track from the library track collection. */
void TrackDAO::removeTrack(int id)
{
    Q_ASSERT(id >= 0);
    QSqlQuery query(m_database);

    //query.prepare("DELETE FROM library WHERE location==" + QString("%1").arg(trackLocationId));
    //Mark the track as deleted!
    query.prepare("UPDATE library "
                  "SET mixxx_deleted=1 "
                  "WHERE id==" + QString("%1").arg(id));
    query.exec();
    //query.finish();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }
}

TrackInfoObject *TrackDAO::getTrackFromDB(QSqlQuery &query) const
{
    TrackInfoObject* track = NULL;
    if (!query.isValid()) {
        //query.exec();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }

    int locationId = -1;
    while (query.next()) {
        track = new TrackInfoObject();
        int trackId = query.value(query.record().indexOf("id")).toInt();

        QString artist = query.value(query.record().indexOf("artist")).toString();
        QString title = query.value(query.record().indexOf("title")).toString();
        QString album = query.value(query.record().indexOf("album")).toString();
        QString year = query.value(query.record().indexOf("year")).toString();
        QString genre = query.value(query.record().indexOf("genre")).toString();
        QString tracknumber = query.value(query.record().indexOf("tracknumber")).toString();
        locationId = query.value(query.record().indexOf("location")).toInt();
        QString comment = query.value(query.record().indexOf("comment")).toString();
        QString url = query.value(query.record().indexOf("url")).toString();
        int duration = query.value(query.record().indexOf("duration")).toInt();
        int length = query.value(query.record().indexOf("length_in_bytes")).toInt();
        int bitrate = query.value(query.record().indexOf("bitrate")).toInt();
        int samplerate = query.value(query.record().indexOf("samplerate")).toInt();
        int cuepoint = query.value(query.record().indexOf("cuepoint")).toInt();
        QString bpm = query.value(query.record().indexOf("bpm")).toString();
        QByteArray* wavesummaryhex = new QByteArray(
            query.value(query.record().indexOf("wavesummaryhex")).toByteArray());
        //int timesplayed = query.value(query.record().indexOf("timesplayed")).toInt();
        int channels = query.value(query.record().indexOf("channels")).toInt();

        track->setArtist(artist);
        track->setTitle(title);
        track->setAlbum(album);
        track->setYear(year);
        track->setGenre(genre);
        track->setTrackNumber(tracknumber);

        track->setComment(comment);
        track->setURL(url);
        track->setDuration(duration);
        track->setLength(length);
        track->setBitrate(bitrate);
        track->setSampleRate(samplerate);
        track->setCuePoint((float)cuepoint);
        track->setBpm(bpm.toFloat());
        track->setWaveSummary(wavesummaryhex, false);
        //track->setTimesPlayed //Doesn't exist wtfbbq
        track->setChannels(channels);

        track->setCuePoints(m_cueDao.getCuesForTrack(trackId));
    }
    //query.finish();
    
    //Get the track location from the track_locations table.
    if (track != NULL) {
        Q_ASSERT(locationId >= 0);
        query.exec("SELECT * FROM track_locations WHERE id=" + QString("%1").arg(locationId));
        while (query.next()) {
            track->setLocation(query.value(query.record().indexOf("location")).toString());
        }
    }
    //query.finish();

    return track;
}

TrackInfoObject *TrackDAO::getTrack(int id) const
{
    QSqlQuery query(m_database);

    query.exec("SELECT * FROM Library WHERE id=" + QString("%1").arg(id));
 	TrackInfoObject* track = getTrackFromDB(query);

    return track;
}

/** Saves a track's info back to the database */
void TrackDAO::updateTrackInDatabase(TrackInfoObject* pTrack)
{
    qDebug() << "Updating track" << pTrack->getInfo() << "in database...";

    QSqlQuery query(m_database);

    //Get the id of the track location, so we can get the Library table's track entry.
    int trackLocationId = -1;
    query.prepare("SELECT * FROM track_locations WHERE location=:location");
    query.bindValue(":location", pTrack->getLocation());
    query.exec();
    while (query.next()) {
        trackLocationId = query.value(query.record().indexOf("id")).toInt();
    }
    if (trackLocationId < 0) //Track was not found in library, so don't try to update...
        return;
    //query.finish();
    //Q_ASSERT(trackLocationId >= 0);

    //Update everything but "location", since that's what we identify the track by.
    query.prepare("UPDATE library "
                  "SET artist=:artist, "
                  "title=:title, album=:album, year=:year, genre=:genre, "
                  "tracknumber=:tracknumber, "
                  "comment=:comment, url=:url, duration=:duration, "
                  "length_in_bytes=:length_in_bytes, "
                  "bitrate=:bitrate, samplerate=:samplerate, cuepoint=:cuepoint, "
                  "bpm=:bpm, wavesummaryhex=:wavesummaryhex, "
                  "channels=:channels "
                  "WHERE location==" + QString("%1").arg(trackLocationId));
    //query.bindValue(":id", 1001);
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":length_in_bytes", pTrack->getLength());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    query.bindValue(":wavesummaryhex", *(pTrack->getWaveSummary()));
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    query.bindValue(":channels", pTrack->getChannels());
    //query.bindValue(":location", pTrack->getLocation());

    query.exec();
    //query.finish();

    query.prepare("SELECT id from library where location = :location_id");
    query.bindValue(":location_id", trackLocationId);
    if (query.exec()) {
        if (query.next()) {
            int trackId = query.value(0).toInt();
            m_cueDao.saveTrackCues(trackId, pTrack);
        }
    } else {
        qDebug() << "Couldn't save cues for track:" << query.lastError();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }
}


void TrackDAO::invalidateTrackLocations(QString directory)
{
    //qDebug() << "invalidateTrackLocations(" << directory << ")";
    m_database.transaction();
    
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        qDebug() << "Couldn't mark tracks in directory" << directory <<  "as needing verification." << query.lastError();
    }

    m_database.commit();
}

void TrackDAO::markTrackLocationAsVerified(QString location)
{
    //qDebug() << "markTrackLocationAsVerified()" << location;
    m_database.transaction();
    
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=0, fs_deleted=0 "
                  "WHERE location=:location"); 
    query.bindValue(":location", location);
    if (!query.exec()) {
        qDebug() << "Couldn't mark track" << location << " as verified." << query.lastError();
    }

    m_database.commit();

}

void TrackDAO::markUnverifiedTracksAsDeleted()
{
    //qDebug() << "markUnverifiedTracksAsDeleted()";
    m_database.transaction();
    
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE needs_verification=1");
    if (!query.exec()) {
        qDebug() << "Couldn't mark unverified tracks as deleted." << query.lastError();
    }

    m_database.commit();
}

void TrackDAO::markTrackLocationsAsDeleted(QString directory)
{
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);             
    if (!query.exec()) {
        qDebug() << "Couldn't mark tracks in" << directory << "as deleted." << query.lastError();
    }
}
