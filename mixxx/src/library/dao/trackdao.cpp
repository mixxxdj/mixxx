
#include <QtDebug>
#include <QtCore>
#include <QtSql>
#include "trackinfoobject.h"
#include "library/dao/trackdao.h"

// The number of tracks to cache in memory at once. Once the n+1'th track is
// created, the TrackDAO's QCache deletes its TrackPointer to the track, which
// allows the track reference count to drop to zero. The track cache basically
// functions to hold a reference to the track so its reference count stays above
// 0.
#define TRACK_CACHE_SIZE 20

TrackDAO::TrackDAO(QSqlDatabase& database, CueDAO& cueDao)
        : m_database(database),
          m_cueDao(cueDao),
          m_trackCache(TRACK_CACHE_SIZE) {

}

TrackDAO::~TrackDAO()
{
}

void TrackDAO::initialize()
{
    qDebug() << "TrackDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

/** Retrieve the track id for the track that's located at "location" on disk.
    @return the track id for the track located at location, or -1 if the track
            is not in the database.
*/
int TrackDAO::getTrackId(QString absoluteFilePath)
{
    //qDebug() << "TrackDAO::getTrackId" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT library.id FROM library INNER JOIN track_locations ON library.location = track_locations.id WHERE track_locations.location=:location");
    query.bindValue(":location", absoluteFilePath);

    if (!query.exec()) {
        qDebug() << query.lastError();
        return -1;
    }

    int libraryTrackId = -1;
    if (query.next()) {
        libraryTrackId = query.value(query.record().indexOf("id")).toInt();
    }
    //query.finish();

    return libraryTrackId;
}

/** Some code (eg. drag and drop) needs to just get a track's location, and it's
    not worth retrieving a whole TrackInfoObject.*/
QString TrackDAO::getTrackLocation(int trackId)
{
    qDebug() << "TrackDAO::getTrackLocation"
             << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    QString trackLocation = "";
    query.prepare("SELECT track_locations.location FROM track_locations INNER JOIN library ON library.location = track_locations.id WHERE library.id=:id");
    query.bindValue(":id", trackId);
    if (!query.exec()) {
        qDebug() << query.lastError();
        return "";
    }
    while (query.next()) {
        trackLocation = query.value(query.record().indexOf("location")).toString();
    }
    //query.finish();

    return trackLocation;
}

/** Check if a track exists in the library table already.
    @param file_location The full path to the track on disk, including the filename.
    @return true if the track is found in the library table, false otherwise.
*/
bool TrackDAO::trackExistsInDatabase(QString absoluteFilePath)
{
    return (getTrackId(absoluteFilePath) != -1);
}

void TrackDAO::saveTrack(TrackPointer track) {

    if (track)
        saveTrack(track.data());
}

void TrackDAO::saveTrack(TrackInfoObject* pTrack) {
    // If track's id is not -1, then update, otherwise add.
    int trackId = pTrack->getId();
    if (trackId != -1) {
        if (pTrack->isDirty()) {
            updateTrack(pTrack);
            m_dirtyTracks.remove(trackId);
            emit(trackClean(trackId));
        } else {
            Q_ASSERT(!m_dirtyTracks.contains(trackId));
            //qDebug() << "Skipping track update for track" << pTrack->getId();
        }
    } else {
        addTrack(pTrack);
    }
}

bool TrackDAO::isDirty(int trackId) {
    return m_dirtyTracks.contains(trackId);
}

void TrackDAO::slotTrackDirty() {
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to comlete.
    TrackInfoObject* pTrack = dynamic_cast<TrackInfoObject*>(sender());
    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            m_dirtyTracks.insert(id);
            emit(trackDirty(id));
        }
    }
}

void TrackDAO::slotTrackChanged() {
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to comlete.
    TrackInfoObject* pTrack = dynamic_cast<TrackInfoObject*>(sender());
    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            emit(trackChanged(id));
        }
    }
}

void TrackDAO::slotTrackSave() {
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The last
    // time it is used is when the track is being deleted (i.e. its reference
    // count has dropped to 0). The TIO is deleted with QObject:deleteLater, so
    // Qt will wait for this slot to comlete.
    TrackInfoObject* pTrack = dynamic_cast<TrackInfoObject*>(sender());
    if (pTrack) {
        saveTrack(pTrack);
    }
}

void TrackDAO::saveDirtyTracks() {
    QHashIterator<int, TrackWeakPointer> it(m_tracks);
    while (it.hasNext()) {
        it.next();
        // Auto-cast from TrackWeakPointer to TrackPointer
        TrackPointer pTrack = it.value();
        if (pTrack && pTrack->isDirty()) {
            saveTrack(pTrack);
        }
    }
}

int TrackDAO::addTrack(QFileInfo& fileInfo) {
    int trackId = -1;
    TrackInfoObject * pTrack = new TrackInfoObject(fileInfo);
    if (pTrack) {
        //Add the song to the database.
        addTrack(pTrack);
        trackId = pTrack->getId();
        delete pTrack;
    }
    return trackId;
}

int TrackDAO::addTrack(QString absoluteFilePath)
{
    QFileInfo fileInfo(absoluteFilePath);
    return addTrack(fileInfo);
}

void TrackDAO::addTrack(TrackInfoObject* pTrack)
{
    QTime time;
    time.start();

    //qDebug() << "TrackDAO::addTrack" << QThread::currentThread() << m_database.connectionName();
 	//qDebug() << "TrackCollection::addTrack(), inserting into DB";
    Q_ASSERT(pTrack); //Why you be giving me NULL pTracks

    //Start the transaction
    m_database.transaction();

    QSqlQuery query(m_database);
    int trackLocationId = -1;

    //Insert the track location into the corresponding table. This will fail silently
    //if the location is already in the table because it has a UNIQUE constraint.
    query.prepare("INSERT INTO track_locations (location, directory, filename, filesize, fs_deleted, needs_verification) "
                  "VALUES (:location, :directory, :filename, :filesize, :fs_deleted, :needs_verification)");
    query.bindValue(":location", pTrack->getLocation());
    query.bindValue(":directory", pTrack->getDirectory());
    query.bindValue(":filename", pTrack->getFilename());
    query.bindValue(":filesize", pTrack->getLength());
    // Should this check pTrack->exists()?
    query.bindValue(":fs_deleted", 0);
    query.bindValue(":needs_verification", 0);

    if (!query.exec()) {
        // Inserting into track_locations failed, so the file already
        // exists. Query for its id.
        query.prepare("SELECT id FROM track_locations WHERE location=:location");
        query.bindValue(":location", pTrack->getLocation());

        if (!query.exec()) {
            // We can't even select this, something is wrong.
            qDebug() << query.lastError();
            m_database.rollback();
            return;
        }
        while (query.next()) {
            trackLocationId = query.value(query.record().indexOf("id")).toInt();
        }
    } else {
        // Inserting succeeded, so just get the last rowid.
        QVariant lastInsert = query.lastInsertId();
        trackLocationId = lastInsert.toInt();
    }

    //Failure of this assert indicates that we were unable to insert the track
    //location into the table AND we could not retrieve the id of that track
    //location from the same table. "It shouldn't happen"... unless I screwed up
    //- Albert :)
    Q_ASSERT(trackLocationId >= 0);

    query.prepare("INSERT INTO library (artist, title, album, year, genre, tracknumber, "
                  "filetype, location, comment, url, duration, "
                  "bitrate, samplerate, cuepoint, bpm, wavesummaryhex, "
                  "datetime_added, "
                  "channels, mixxx_deleted, header_parsed) "
                  "VALUES (:artist, "
                  ":title, :album, :year, :genre, :tracknumber, "
                  ":filetype, :location, :comment, :url, :duration, "
                  ":bitrate, :samplerate, :cuepoint, :bpm, :wavesummaryhex, "
                  ":datetime_added, "
                  ":channels, :mixxx_deleted, :header_parsed)");
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":filetype", pTrack->getType());
    query.bindValue(":location", trackLocationId);
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    const QByteArray* pWaveSummary = pTrack->getWaveSummary();
    if (pWaveSummary) //Avoid null pointer deref
        query.bindValue(":wavesummaryhex", *pWaveSummary);
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    query.bindValue(":datetime_added", pTrack->getCreateDate());
    query.bindValue(":channels", pTrack->getChannels());
    query.bindValue(":mixxx_deleted", 0);
    query.bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);

    int trackId = -1;

    if (!query.exec())
    {
        qDebug() << "Failed to INSERT new track into library"
                 << __FILE__ << __LINE__ << query.lastError();
        m_database.rollback();
        return;
    } else {
        // Inserting succeeded, so just get the last rowid.
        trackId = query.lastInsertId().toInt();
    }
    //query.finish();

    Q_ASSERT(trackId >= 0);

    if (trackId >= 0) {
        m_cueDao.saveTrackCues(trackId, pTrack);
    } else {
        qDebug() << "Could not get track ID to save the track cue points:"
                 << query.lastError();
    }

    pTrack->setId(trackId);
    pTrack->setDirty(false);

    //If addTrack() is called on a track that already exists in the library but
    //has been "removed" (ie. mixxx_deleted is 1), then the above INSERT will
    //fail silently. What we really want to do is just mark the track as
    //undeleted, by setting mixxx_deleted to 0.  addTrack() will not get called
    //on files that are already in the library during a rescan (even if
    //mixxx_deleted=1). However, this function WILL get called when a track is
    //dragged and dropped onto the library or when manually imported from the
    //File... menu.  This allows people to re-add tracks that they "removed"...
    query.prepare("UPDATE library "
                  "SET mixxx_deleted=0 "
                  "WHERE id = " + QString("%1").arg(trackId));
    if (!query.exec()) {
        qDebug() << "Failed to set track" << trackId << "as undeleted" << query.lastError();
    }
    //query.finish();

    //Commit the transaction
    m_database.commit();
    //qDebug() << "addTrack took" << time.elapsed() << "ms";
}

  /** Removes a track from the library track collection. */
void TrackDAO::removeTrack(int id)
{
    //qDebug() << "TrackDAO::removeTrack" << QThread::currentThread() << m_database.connectionName();
    Q_ASSERT(id >= 0);
    QSqlQuery query(m_database);

    //Mark the track as deleted!
    query.prepare("UPDATE library "
                  "SET mixxx_deleted=1 "
                  "WHERE id = " + QString("%1").arg(id));
    query.exec();
    //query.finish();

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }
}

// static
void TrackDAO::deleteTrack(TrackInfoObject* pTrack) {
    Q_ASSERT(pTrack);

    //qDebug() << "Got deletion call for track" << pTrack << "ID" << pTrack->getId() << pTrack->getInfo();

    // Save dirty tracks.
    pTrack->save();

    // if (iId != -1 && isDirty(iId)) {
    //     saveTrack(track);
    // }

    // Now Qt will delete it in the event loop.
    pTrack->deleteLater();
}

TrackPointer TrackDAO::getTrackFromDB(QSqlQuery &query) const
{
    if (!query.isValid()) {
        //query.exec();
    }

    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        qDebug() << query.lastError();
    }

    int locationId = -1;
    while (query.next()) {
        int trackId = query.value(query.record().indexOf("id")).toInt();

        QString artist = query.value(query.record().indexOf("artist")).toString();
        QString title = query.value(query.record().indexOf("title")).toString();
        QString album = query.value(query.record().indexOf("album")).toString();
        QString year = query.value(query.record().indexOf("year")).toString();
        QString genre = query.value(query.record().indexOf("genre")).toString();
        QString tracknumber = query.value(query.record().indexOf("tracknumber")).toString();
        QString comment = query.value(query.record().indexOf("comment")).toString();
        QString url = query.value(query.record().indexOf("url")).toString();
        int duration = query.value(query.record().indexOf("duration")).toInt();
        int bitrate = query.value(query.record().indexOf("bitrate")).toInt();
        int samplerate = query.value(query.record().indexOf("samplerate")).toInt();
        int cuepoint = query.value(query.record().indexOf("cuepoint")).toInt();
        QString bpm = query.value(query.record().indexOf("bpm")).toString();
        QByteArray* wavesummaryhex = new QByteArray(
            query.value(query.record().indexOf("wavesummaryhex")).toByteArray());
        //int timesplayed = query.value(query.record().indexOf("timesplayed")).toInt();
        int channels = query.value(query.record().indexOf("channels")).toInt();
        int filesize = query.value(query.record().indexOf("filesize")).toInt();
        QString filetype = query.value(query.record().indexOf("filetype")).toString();
        QString location = query.value(query.record().indexOf("location")).toString();
        bool header_parsed = query.value(query.record().indexOf("header_parsed")).toBool();

        TrackInfoObject* track = new TrackInfoObject(location, false);

        // TIO already stats the file to see if it exists, what its length is,
        // etc. So don't bother setting it.
        //track->setLength(filesize);

        track->setId(trackId);
        track->setArtist(artist);
        track->setTitle(title);
        track->setAlbum(album);
        track->setYear(year);
        track->setGenre(genre);
        track->setTrackNumber(tracknumber);

        track->setComment(comment);
        track->setURL(url);
        track->setDuration(duration);
        track->setBitrate(bitrate);
        track->setSampleRate(samplerate);
        track->setCuePoint((float)cuepoint);
        track->setBpm(bpm.toFloat());
        track->setWaveSummary(wavesummaryhex, false);
        delete wavesummaryhex;
        //track->setTimesPlayed //Doesn't exist wtfbbq
        track->setChannels(channels);
        track->setType(filetype);
        track->setLocation(location);
        track->setHeaderParsed(header_parsed);

        track->setCuePoints(m_cueDao.getCuesForTrack(trackId));
        track->setDirty(false);

        // Listen to dirty and changed signals
        connect(track, SIGNAL(dirty()),
                this, SLOT(slotTrackDirty()));
        connect(track, SIGNAL(changed()),
                this, SLOT(slotTrackChanged()));
        connect(track, SIGNAL(save()),
                this, SLOT(slotTrackSave()));

        TrackPointer pTrack = TrackPointer(track, this->deleteTrack);

        // Automatic conversion to a weak pointer
        m_tracks[trackId] = pTrack;
        m_trackCache.insert(trackId, new TrackPointer(pTrack));

        // If the header hasn't been parsed, parse it but only after we set the
        // track clean and hooked it up to the track cache, because this will
        // dirty it.
        if (!header_parsed) {
            pTrack->parse();
        }

        return pTrack;
    }
    //query.finish();

    return TrackPointer();
}

TrackPointer TrackDAO::getTrack(int id) const
{
    //qDebug() << "TrackDAO::getTrack" << QThread::currentThread() << m_database.connectionName();


    // If the track cache contains the track, use it to get a strong reference
    // to the track. We do this first so that the QCache keeps track of the
    // least-recently-used track so that it expires them intelligently.
    if (m_trackCache.contains(id)) {
        TrackPointer& pTrack = *m_trackCache[id];

        // If the strong reference is still valid (it should be), then return
        // it. Otherwise query the DB for the track.
        if (pTrack)
            return pTrack;
    }

    // Next, check the weak-reference cache to see if the track was ever loaded
    // into memory. It's possible that something is currently using this track,
    // so its reference count is non-zero despite it not being present in the
    // track cache. m_tracks is a map of weak pointers to the tracks.
    if (m_tracks.contains(id)) {
        //qDebug() << "Returning cached TIO for track" << id;
        TrackPointer pTrack = m_tracks[id];

        // If the pointer to the cached copy is still valid, return
        // it. Otherwise, re-query the DB for the track.
        if (pTrack)
            return pTrack;
    }

    QTime time;
    time.start();
    QSqlQuery query(m_database);

    query.prepare("SELECT library.id, artist, title, album, year, genre, tracknumber, filetype, track_locations.location as location, track_locations.filesize as filesize, comment, url, duration, bitrate, samplerate, cuepoint, bpm, wavesummaryhex, channels, header_parsed FROM Library INNER JOIN track_locations ON library.location = track_locations.id WHERE library.id=" + QString("%1").arg(id));
    TrackPointer pTrack;

    if (query.exec()) {
         pTrack = getTrackFromDB(query);
    } else {
        qDebug() << QString("getTrack(%1)").arg(id) << query.lastError();
    }
    //qDebug() << "getTrack hit the database, took " << time.elapsed() << "ms";

    return pTrack;
}

/** Saves a track's info back to the database */
void TrackDAO::updateTrack(TrackInfoObject* pTrack)
{
    m_database.transaction();
    QTime time;
    time.start();
    Q_ASSERT(pTrack);
    //qDebug() << "TrackDAO::updateTrackInDatabase" << QThread::currentThread() << m_database.connectionName();

    //qDebug() << "Updating track" << pTrack->getInfo() << "in database...";

    int trackId = pTrack->getId();
    Q_ASSERT(trackId >= 0);

    QSqlQuery query(m_database);

    //Update everything but "location", since that's what we identify the track by.
    query.prepare("UPDATE library "
                  "SET artist=:artist, "
                  "title=:title, album=:album, year=:year, genre=:genre, "
                  "filetype=:filetype, tracknumber=:tracknumber, "
                  "comment=:comment, url=:url, duration=:duration, "
                  "bitrate=:bitrate, samplerate=:samplerate, cuepoint=:cuepoint, "
                  "bpm=:bpm, wavesummaryhex=:wavesummaryhex, "
                  "channels=:channels, header_parsed=:header_parsed "
                  "WHERE id="+QString("%1").arg(trackId));
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":filetype", pTrack->getType());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm", pTrack->getBpm());
    const QByteArray* pWaveSummary = pTrack->getWaveSummary();
    if (pWaveSummary) //Avoid null pointer deref
        query.bindValue(":wavesummaryhex", *pWaveSummary);
    //query.bindValue(":timesplayed", pTrack->getCuePoint());
    query.bindValue(":channels", pTrack->getChannels());
    query.bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);
    //query.bindValue(":location", pTrack->getLocation());

    if (!query.exec()) {
        qDebug() << query.lastError();
        m_database.rollback();
        return;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "updateTrack had no effect: trackId" << trackId << "invalid";
        m_database.rollback();
        return;
    }

    //query.finish();

    //qDebug() << "Update track took : " << time.elapsed() << "ms. Now updating cues";
    time.start();
    m_cueDao.saveTrackCues(trackId, pTrack);
    m_database.commit();

    //qDebug() << "Update track in database took: " << time.elapsed() << "ms";
    time.start();
    pTrack->setDirty(false);
    //qDebug() << "Dirtying track took: " << time.elapsed() << "ms";
}

/** Mark all the tracks whose paths begin with libraryPath as invalid.
    That means we'll need to later check that those tracks actually
    (still) exist as part of the library scanning procedure. */
void TrackDAO::invalidateTrackLocationsInLibrary(QString libraryPath)
{
    //qDebug() << "TrackDAO::invalidateTrackLocations" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "invalidateTrackLocations(" << libraryPath << ")";
    libraryPath += "%"; //Add wildcard to SQL query to match subdirectories!

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=1 "
                  "WHERE directory LIKE :directory");
    query.bindValue(":directory", libraryPath);
    if (!query.exec()) {
        qDebug() << "Couldn't mark tracks in directory" << libraryPath <<  "as needing verification." << query.lastError();
    }
}

void TrackDAO::markTrackLocationAsVerified(QString location)
{
    //qDebug() << "TrackDAO::markTrackLocationAsVerified" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "markTrackLocationAsVerified()" << location;

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=0, fs_deleted=0 "
                  "WHERE location=:location");
    query.bindValue(":location", location);
    if (!query.exec()) {
        qDebug() << "Couldn't mark track" << location << " as verified." << query.lastError();
    }
}

void TrackDAO::markTracksInDirectoryAsVerified(QString directory)
{
    //qDebug() << "TrackDAO::markTracksInDirectoryAsVerified" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "markTracksInDirectoryAsVerified()" << directory;

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=0, fs_deleted=0 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        qDebug() << "Couldn't mark tracks in" << directory << " as verified." << query.lastError();
    }
}

void TrackDAO::markUnverifiedTracksAsDeleted()
{
    //qDebug() << "TrackDAO::markUnverifiedTracksAsDeleted" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "markUnverifiedTracksAsDeleted()";

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE needs_verification=1");
    if (!query.exec()) {
        qDebug() << "Couldn't mark unverified tracks as deleted." << query.lastError();
    }

}

void TrackDAO::markTrackLocationsAsDeleted(QString directory)
{
    //qDebug() << "TrackDAO::markTrackLocationsAsDeleted" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        qDebug() << "Couldn't mark tracks in" << directory << "as deleted." << query.lastError();
    }
}

/** Look for moved files. Look for files that have been marked as "deleted on disk"
    and see if another "file" with the same name and filesize exists in the track_locations
    table. That means the file has moved instead of being deleted outright, and so
    we can salvage your existing metadata that you have in your DB (like cue points, etc.). */
void TrackDAO::detectMovedFiles()
{
    //This function should not start a transaction on it's own!
    //When it's called from libraryscanner.cpp, there already is a transaction
    //started!

    QSqlQuery query(m_database);
    QSqlQuery query2(m_database);
    int oldTrackLocationId = -1;
    int newTrackLocationId = -1;
    QString filename;
    int fileSize;

    query.prepare("SELECT id, filename, filesize FROM track_locations WHERE fs_deleted=1");
    query.exec();

    //For each track that's been "deleted" on disk...
    while (query.next()) {
        newTrackLocationId = -1; //Reset this var
        oldTrackLocationId = query.value(query.record().indexOf("id")).toInt();
        filename = query.value(query.record().indexOf("filename")).toString();
        fileSize = query.value(query.record().indexOf("filesize")).toInt();

        query2.prepare("SELECT id FROM track_locations WHERE "
                       "fs_deleted=0 AND "
                       "filename=:filename AND "
                       "filesize=:filesize");
        query2.bindValue(":filename", filename);
        query2.bindValue(":filesize", fileSize);
        Q_ASSERT(query2.exec());

        Q_ASSERT(query2.size() <= 1); //WTF duplicate tracks?
        while (query2.next())
        {
            newTrackLocationId = query2.value(query2.record().indexOf("id")).toInt();
        }

        //If we found a moved track...
        if (newTrackLocationId >= 0)
        {
            qDebug() << "Found moved track!" << filename;

            //Remove old row from track_locations table
            query2.prepare("DELETE FROM track_locations WHERE "
                           "id=:id");
            query2.bindValue(":id", oldTrackLocationId);
            Q_ASSERT(query2.exec());

            //The library scanner will have added a new row to the Library
            //table which corresponds to the track in the new location. We need
            //to remove that so we don't end up with two rows in the library table
            //for the same track.
            query2.prepare("DELETE FROM library WHERE "
                           "location=:location");
            query2.bindValue(":location", newTrackLocationId);
            Q_ASSERT(query2.exec());

            //Update the location foreign key for the existing row in the library table
            //to point to the correct row in the track_locations table.
            query2.prepare("UPDATE library "
                           "SET location=:newloc WHERE location=:oldloc");
            query2.bindValue(":newloc", newTrackLocationId);
            query2.bindValue(":oldloc", oldTrackLocationId);
            Q_ASSERT(query2.exec());
        }
    }
}

void TrackDAO::clearCache()
{
    m_trackCache.clear();
}
