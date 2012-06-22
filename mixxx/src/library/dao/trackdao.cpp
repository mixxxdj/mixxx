#include <QtDebug>
#include <QtCore>
#include <QtSql>

#include "library/dao/trackdao.h"

#include "audiotagger.h"
#include "library/queryutil.h"
#include "soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "trackinfoobject.h"

// The number of tracks to cache in memory at once. Once the n+1'th track is
// created, the TrackDAO's QCache deletes its TrackPointer to the track, which
// allows the track reference count to drop to zero. The track cache basically
// functions to hold a reference to the track so its reference count stays above
// 0.
#define TRACK_CACHE_SIZE 5

TrackDAO::TrackDAO(QSqlDatabase& database,
                   CueDAO& cueDao,
                   PlaylistDAO& playlistDao,
                   CrateDAO& crateDao,
                   AnalysisDao& analysisDao,
                   ConfigObject<ConfigValue> * pConfig)
        : m_database(database),
          m_cueDao(cueDao),
          m_playlistDao(playlistDao),
          m_crateDao(crateDao),
          m_analysisDao(analysisDao),
          m_pConfig(pConfig),
          m_trackCache(TRACK_CACHE_SIZE) {
}

void TrackDAO::finish() {
    // Save all tracks that haven't been saved yet.
    saveDirtyTracks();
    //clear out played information on exit
    //crash prevention: if mixxx crashes, played information will be maintained
    qDebug() << "Clearing played information for this session";
    QSqlQuery query(m_database);
    if (!query.exec("UPDATE library SET played=0")) {
        LOG_FAILED_QUERY(query)
                << "Error clearing played value";
    }
}

TrackDAO::~TrackDAO() {
    qDebug() << "~TrackDAO()";
}

void TrackDAO::initialize() {
    qDebug() << "TrackDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

/** Retrieve the track id for the track that's located at "location" on disk.
    @return the track id for the track located at location, or -1 if the track
            is not in the database.
*/
int TrackDAO::getTrackId(QString absoluteFilePath) {
    //qDebug() << "TrackDAO::getTrackId" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare("SELECT library.id FROM library INNER JOIN track_locations ON library.location = track_locations.id WHERE track_locations.location=:location");
    query.bindValue(":location", absoluteFilePath);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    int libraryTrackId = -1;
    if (query.next()) {
        libraryTrackId = query.value(query.record().indexOf("id")).toInt();
    }

    return libraryTrackId;
}

// Some code (eg. drag and drop) needs to just get a track's location, and it's
// not worth retrieving a whole TrackInfoObject.
QString TrackDAO::getTrackLocation(int trackId) {
    qDebug() << "TrackDAO::getTrackLocation"
             << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    QString trackLocation = "";
    query.prepare("SELECT track_locations.location FROM track_locations INNER JOIN library ON library.location = track_locations.id WHERE library.id=:id");
    query.bindValue(":id", trackId);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return "";
    }
    while (query.next()) {
        trackLocation = query.value(query.record().indexOf("location")).toString();
    }

    return trackLocation;
}

/** Check if a track exists in the library table already.
    @param file_location The full path to the track on disk, including the filename.
    @return true if the track is found in the library table, false otherwise.
*/
bool TrackDAO::trackExistsInDatabase(QString absoluteFilePath) {
    return (getTrackId(absoluteFilePath) != -1);
}

void TrackDAO::saveTrack(TrackPointer track) {
    if (track) {
        saveTrack(track.data());
    }
}

void TrackDAO::saveTrack(TrackInfoObject* pTrack) {
    if (!pTrack) {
        qWarning() << "TrackDAO::saveTrack() was given NULL track.";
    }
    //qDebug() << "TrackDAO::saveTrack" << pTrack->getId() << pTrack->getInfo();
    // If track's id is not -1, then update, otherwise add.
    int trackId = pTrack->getId();
    if (trackId != -1) {
        if (pTrack->isDirty()) {
            if (!m_dirtyTracks.contains(trackId)) {
                qDebug() << "WARNING: Inconsistent state in TrackDAO. Track is dirty while TrackDAO thinks it is clean.";
            }

            //qDebug() << this << "Dirty tracks before claen save:" << m_dirtyTracks.size();
            //qDebug() << "TrackDAO::saveTrack. Dirty. Calling update";
            updateTrack(pTrack);

            // Write audio meta data, if enabled in the preferences
            writeAudioMetaData(pTrack);

            //qDebug() << this << "Dirty tracks remaining after clean save:" << m_dirtyTracks.size();
        } else {
            //qDebug() << "TrackDAO::saveTrack. Not Dirty";
            //qDebug() << this << "Dirty tracks remaining:" << m_dirtyTracks.size();

            // Q_ASSERT(!m_dirtyTracks.contains(trackId));
            if (m_dirtyTracks.contains(trackId)) {
                qDebug() << "WARNING: Inconsistent state in TrackDAO. Track is clean while TrackDAO thinks it is dirty. Correcting.";
                m_dirtyTracks.remove(trackId);
            }

            //qDebug() << "Skipping track update for track" << pTrack->getId();
        }
    } else {
        addTrack(pTrack, false);
    }
}

bool TrackDAO::isDirty(int trackId) {
    return m_dirtyTracks.contains(trackId);
}

void TrackDAO::slotTrackDirty(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackDirty" << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to comlete.
    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            m_dirtyTracks.insert(id);
            emit(trackDirty(id));
        }
    }
}

void TrackDAO::slotTrackClean(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackClean" << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to comlete.

    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            m_dirtyTracks.remove(id);
            emit(trackClean(id));
        }
    }
}

void TrackDAO::slotTrackChanged(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackChanged" << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to comlete.
    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            emit(trackChanged(id));
        }
    }
}

void TrackDAO::slotTrackSave(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackSave" << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The last
    // time it is used is when the track is being deleted (i.e. its reference
    // count has dropped to 0). The TIO is deleted with QObject:deleteLater, so
    // Qt will wait for this slot to comlete.
    if (pTrack) {
        saveTrack(pTrack);
    }
}

void TrackDAO::saveDirtyTracks() {
    qDebug() << "TrackDAO::saveDirtyTracks()";
    QHashIterator<int, TrackWeakPointer> it(m_tracks);
    while (it.hasNext()) {
        it.next();
        // Auto-cast from TrackWeakPointer to TrackPointer
        TrackPointer pTrack = it.value();
        if (pTrack && pTrack->isDirty()) {
            saveTrack(pTrack);
        }
    }
    clearCache();
}

void TrackDAO::prepareTrackLocationsInsert(QSqlQuery& query) {
    query.prepare("INSERT INTO track_locations (location, directory, filename, filesize, fs_deleted, needs_verification) "
                  "VALUES (:location, :directory, :filename, :filesize, :fs_deleted, :needs_verification)");
}

void TrackDAO::bindTrackToTrackLocationsInsert(QSqlQuery& query, TrackInfoObject* pTrack) {
    query.bindValue(":location", pTrack->getLocation());
    query.bindValue(":directory", pTrack->getDirectory());
    query.bindValue(":filename", pTrack->getFilename());
    query.bindValue(":filesize", pTrack->getLength());
    // Should this check pTrack->exists()?
    query.bindValue(":fs_deleted", 0);
    query.bindValue(":needs_verification", 0);
}

void TrackDAO::prepareLibraryInsert(QSqlQuery& query) {
    query.prepare("INSERT INTO library (artist, title, album, year, genre, composer, "
				  "tracknumber, filetype, location, comment, url, duration, rating, key, "
                  "bitrate, samplerate, cuepoint, bpm, replaygain, wavesummaryhex, "
                  "timesplayed, "
                  "channels, mixxx_deleted, header_parsed, beats_version, beats_sub_version, beats, bpm_lock) "
                  "VALUES (:artist, "
                  ":title, :album, :year, :genre, :composer, :tracknumber, "
                  ":filetype, :location, :comment, :url, :duration, :rating, :key, "
                  ":bitrate, :samplerate, :cuepoint, :bpm, :replaygain, :wavesummaryhex, "
                  ":timesplayed, "
                  ":channels, :mixxx_deleted, :header_parsed, :beats_version, :beats_sub_version, :beats, :bpm_lock)");
}

void TrackDAO::bindTrackToLibraryInsert(
    QSqlQuery& query, TrackInfoObject* pTrack, int trackLocationId) {
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":composer", pTrack->getComposer());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":filetype", pTrack->getType());
    query.bindValue(":location", trackLocationId);
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":rating", pTrack->getRating());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());
    query.bindValue(":bpm_lock", pTrack->hasBpmLock()? 1 : 0);

    query.bindValue(":replaygain", pTrack->getReplayGain());
    query.bindValue(":key", pTrack->getKey());

    // We no longer store the wavesummary in the library table.
    query.bindValue(":wavesummaryhex", QVariant(QVariant::ByteArray));

    query.bindValue(":timesplayed", pTrack->getTimesPlayed());
    //query.bindValue(":datetime_added", pTrack->getDateAdded());
    query.bindValue(":channels", pTrack->getChannels());
    query.bindValue(":mixxx_deleted", 0);
    query.bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);

    const QByteArray* pBeatsBlob = NULL;
    QString blobVersion = "";
    BeatsPointer pBeats = pTrack->getBeats();
    // Fall back on cached BPM
    double dBpm = pTrack->getBpm();

    if (pBeats) {
        pBeatsBlob = pBeats->toByteArray();
        blobVersion = pBeats->getVersion();
        dBpm = pBeats->getBpm();
    }

    query.bindValue(":bpm", dBpm);
    query.bindValue(":beats_version", blobVersion);
    query.bindValue(":beats", pBeatsBlob ? *pBeatsBlob : QVariant(QVariant::ByteArray));
    delete pBeatsBlob;
}

void TrackDAO::addTracks(QList<TrackInfoObject*> tracksToAdd, bool unremove) {
    QSet<int> tracksAddedSet;
    QTime time;
    time.start();

    // Start the transaction
    ScopedTransaction transaction(m_database);

    QSqlQuery query(m_database);
    QSqlQuery query_finder(m_database);
    query_finder.prepare("SELECT id FROM track_locations WHERE location=:location");

    // Major time saver for having this outside the loop
    prepareTrackLocationsInsert(query);

    QStringList trackLocationIds;
    foreach (TrackInfoObject* pTrack, tracksToAdd) {
        if (pTrack == NULL || !isTrackFormatSupported(pTrack)) {
            // TODO(XXX) provide some kind of error code on a per-track basis.
            continue;
        }
        bindTrackToTrackLocationsInsert(query, pTrack);

        int trackLocationId = -1;
        if (!query.exec()) {
            qDebug() << "Location " << pTrack->getLocation() << " is already in the DB";
            query_finder.bindValue(":location", pTrack->getLocation());

            if (!query_finder.exec()) {
                // We can't even select this, something is wrong. Skip this
                // track -- maybe we'll have luck with others.
                LOG_FAILED_QUERY(query_finder)
                        << "Can't find track location ID after failing to insert. Something is wrong.";
                continue;
            }
            while (query_finder.next()) {
                trackLocationId = query_finder.value(query_finder.record().indexOf("id")).toInt();
            }
        } else {
            // Inserting succeeded, so just get the last rowid.
            QVariant lastInsert = query.lastInsertId();
            trackLocationId = lastInsert.toInt();
        }

        // To save time on future queries, setId the trackLocationId on the
        // track. This takes advantage of the fact that I know the
        // LibraryScanner doesn't use these tracks for anything. rryan 9/2010
        pTrack->setId(trackLocationId);
        trackLocationIds.append(QString::number(trackLocationId));
    }

    // Look up pre-existing library records for the track location ids.
    QSqlQuery track_lookup(m_database);
    // Mapping of track library record id to mixxx_deleted field.
    QHash<int, QPair<int, bool> > tracksPresent;
    track_lookup.prepare(
        QString("SELECT location, id, mixxx_deleted from library WHERE location IN (%1)")
        .arg(trackLocationIds.join(",")));

    if (!track_lookup.exec()) {
        LOG_FAILED_QUERY(track_lookup)
                << "Failed to lookup existing tracks:";
    } else {
        QSqlRecord track_lookup_record = track_lookup.record();
        int locationIdColumn = track_lookup_record.indexOf("location");
        int idColumn = track_lookup_record.indexOf("id");
        int mixxxDeletedColumn = track_lookup_record.indexOf("mixxx_deleted");
        while (track_lookup.next()) {
            int locationId = track_lookup.value(locationIdColumn).toInt();
            int trackId = track_lookup.value(idColumn).toInt();
            bool removed = track_lookup.value(mixxxDeletedColumn).toBool();
            tracksPresent[locationId] = QPair<int, bool>(trackId, removed);
        }
    }

    // Major time saver for having this outside the loop
    prepareLibraryInsert(query);

    foreach (TrackInfoObject* pTrack, tracksToAdd) {
        // Skip tracks that did not make it past the previous part.
        if (pTrack == NULL || pTrack->getId() < 0) {
            continue;
        }

        // Immediately undo the hack we did above so we do not accidentally
        // leave the ID incorrectly set.
        int locationId = pTrack->getId();
        pTrack->setId(-1);

        // Skip tracks that are already in the database. Optionally unremove
        // them.
        QHash<int, QPair<int, bool> >::const_iterator it = tracksPresent.find(locationId);
        if (it != tracksPresent.end()) {
            int trackId = it.value().first;
            bool removed = it.value().second;
            if (removed && unremove) {
                QSqlQuery unremove_query(m_database);
                unremove_query.prepare("UPDATE library SET mixxx_deleted = 0 WHERE id = :id");
                unremove_query.bindValue(":id", trackId);
                if (!unremove_query.exec()) {
                    LOG_FAILED_QUERY(unremove_query)
                            << "Could not unremove track" << trackId;
                } else {
                    tracksAddedSet.insert(trackId);
                }
            }

            // Regardless of whether we unremoved this track or not -- it's
            // already in the library and so we need to skip it. Set the track's
            // trackId so the caller can know it. TODO(XXX) this is a little
            // weird because the track has whatever metadata the caller supplied
            // and that metadata may differ from what is already in the
            // database. I'm ignoring this corner case. rryan 10/2011
            pTrack->setId(trackId);
            continue;
        }

        bindTrackToLibraryInsert(query, pTrack, locationId);

        if (!query.exec()) {
            // We failed to insert the track. Maybe it is already in the library
            // but marked deleted? Skip this track.
            LOG_FAILED_QUERY(query)
                    << "Failed to INSERT new track into library:"
                    << pTrack->getFilename();
            continue;
        }
        int trackId = query.lastInsertId().toInt();
        pTrack->setId(trackId);
        m_analysisDao.saveTrackAnalyses(pTrack);
        m_cueDao.saveTrackCues(trackId, pTrack);
        pTrack->setDirty(false);
        tracksAddedSet.insert(trackId);
    }

    transaction.commit();

    qDebug() << this << "addTracks took" << time.elapsed() << "ms to add"
             << tracksAddedSet.size() << "tracks";
    if (tracksAddedSet.size() > 0) {
        emit(tracksAdded(tracksAddedSet));
    }
}

int TrackDAO::addTrack(QFileInfo& fileInfo, bool unremove) {
    int trackId = -1;
    TrackInfoObject * pTrack = new TrackInfoObject(fileInfo);
    if (pTrack) {
        // Add the song to the database.
        addTrack(pTrack, unremove);
        trackId = pTrack->getId();
        delete pTrack;
    }
    return trackId;
}

QList<int> TrackDAO::addTracks(QList<QFileInfo> fileInfoList, bool unremove) {
    QList<int> trackIDs;

    //create the list of TrackInfoObjects from the fileInfoList
    QList<TrackInfoObject*> pTrackList;
    QMutableListIterator<QFileInfo> it(fileInfoList);
    while (it.hasNext()) {
        QFileInfo& info = it.next();
        pTrackList.append(new TrackInfoObject(info));
    }

    addTracks(pTrackList, unremove);

    foreach (TrackInfoObject* pTrack, pTrackList) {
        int trackID = pTrack->getId();
        if (trackID >= 0) {
            trackIDs.append(trackID);
        }
        delete pTrack;
    }
    return trackIDs;
}

int TrackDAO::addTrack(QString absoluteFilePath, bool unremove)
{
    QFileInfo fileInfo(absoluteFilePath);
    return addTrack(fileInfo, unremove);
}

void TrackDAO::addTrack(TrackInfoObject* pTrack, bool unremove) {
    QList<TrackInfoObject*> tracksToAdd;
    tracksToAdd.push_back(pTrack);
    addTracks(tracksToAdd, unremove);
}

void TrackDAO::hideTracks(QList<int> ids) {
    QStringList idList;
    foreach (int id, ids) {
        idList.append(QString::number(id));
    }

    QSqlQuery query(m_database);
    query.prepare(QString("UPDATE library SET mixxx_deleted=1 WHERE id in (%1)")
                  .arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // This is signal is received by beasetrackcache to remove the tracks from cache
    QSet<int> tracksRemovedSet = QSet<int>::fromList(ids);
    emit(tracksRemoved(tracksRemovedSet));
}

// If a track has been manually "hidden" from Mixxx's library by the user via
// Mixxx's interface, this lets you add it back. When a track is hidden,
// mixxx_deleted in the DB gets set to 1. This clears that, and makes it show
// up in the library views again.
// This function should get called if you drag-and-drop a file that's been
// "hidden" from Mixxx back into the library view.
void TrackDAO::unhideTracks(QList<int> ids) {
    QStringList idList;
    foreach (int id, ids) {
        idList.append(QString::number(id));
    }

    QSqlQuery query(m_database);
    query.prepare(QString("UPDATE library SET mixxx_deleted=0 "
                  "WHERE id in (%1)").arg(idList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    QSet<int> tracksAddedSet = QSet<int>::fromList(ids);
    emit(tracksAdded(tracksAddedSet));
}

// Warning, purge cannot be undone check before if there is no reference to this
// track id's on other library tables
void TrackDAO::purgeTracks(QList<int> ids) {
    if (ids.empty()) {
        return;
    }

    QStringList idList;
    foreach (int id, ids) {
        idList << QString::number(id);
    }
    QString idListJoined = idList.join(",");

    ScopedTransaction transaction(m_database);

    QSqlQuery query(m_database);
    query.prepare(QString("SELECT track_locations.location, track_locations.directory FROM "
                          "track_locations INNER JOIN library ON library.location = "
                          "track_locations.id WHERE library.id in (%1)").arg(idListJoined));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    FieldEscaper escaper(m_database);
    QStringList locationList;
    QSet<QString> dirs;
    while (query.next()) {
        QString filePath = query.value(query.record().indexOf("location")).toString();
        locationList << escaper.escapeString(filePath);
        QString directory = query.value(query.record().indexOf("directory")).toString();
        dirs.insert(directory);
    }

    QStringList dirList;
    for (QSet<QString>::const_iterator it = dirs.constBegin();
         it != dirs.constEnd(); ++it) {
        dirList << escaper.escapeString(*it);
    }

    if (locationList.empty()) {
        LOG_FAILED_QUERY(query);
    }

    // Remove location from track_locations table
    query.prepare(QString("DELETE FROM track_locations "
                          "WHERE location in (%1)").arg(locationList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Remove Track from library table
    query.prepare(QString("DELETE FROM library "
                          "WHERE id in (%1)").arg(idListJoined));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // mark LibraryHash with needs_verification and invalidate the hash
    // in case the file was not deleted to detect it on a rescan
    // TODO(XXX) delegate to libraryHashDAO
    query.prepare(QString("UPDATE LibraryHashes SET needs_verification=1, "
                          "hash=-1 WHERE directory_path in (%1)").arg(dirList.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // TODO(XXX) Not sure if we should check any of these for errors or just not
    // care if there were errors and commit anyway.
    if (query.lastError().isValid()) {
        return;
    }
    transaction.commit();

    // also need to clean playlists, crates, cues and track_analyses

    m_cueDao.deleteCuesForTracks(ids);
    m_playlistDao.removeTracksFromPlaylists(ids);
    m_crateDao.removeTracksFromCrates(ids);
    m_analysisDao.deleteAnalysises(ids);

    QSet<int> tracksRemovedSet = QSet<int>::fromList(ids);
    emit(tracksRemoved(tracksRemovedSet));
}

// deleter of the TrackInfoObject, for delete a Track from Library use hide or purge
// static
void TrackDAO::deleteTrack(TrackInfoObject* pTrack) {
    Q_ASSERT(pTrack);
    //qDebug() << "Garbage Collecting" << pTrack << "ID" << pTrack->getId() << pTrack->getInfo();

    // Save the track if it is dirty.
    pTrack->doSave();

    // Now Qt will delete it in the event loop.
    pTrack->deleteLater();
}

TrackPointer TrackDAO::getTrackFromDB(QSqlQuery &query) const {
    //Print out any SQL error, if there was one.
    if (query.lastError().isValid()) {
        LOG_FAILED_QUERY(query);
    }

    while (query.next()) {
        // Good god! Assign query.record() to a freaking variable!
        int trackId = query.value(query.record().indexOf("id")).toInt();
        QString artist = query.value(query.record().indexOf("artist")).toString();
        QString title = query.value(query.record().indexOf("title")).toString();
        QString album = query.value(query.record().indexOf("album")).toString();
        QString year = query.value(query.record().indexOf("year")).toString();
        QString genre = query.value(query.record().indexOf("genre")).toString();
        QString composer = query.value(query.record().indexOf("composer")).toString();
        QString tracknumber = query.value(query.record().indexOf("tracknumber")).toString();
        QString comment = query.value(query.record().indexOf("comment")).toString();
        QString url = query.value(query.record().indexOf("url")).toString();
        QString key = query.value(query.record().indexOf("key")).toString();
        int duration = query.value(query.record().indexOf("duration")).toInt();
        int bitrate = query.value(query.record().indexOf("bitrate")).toInt();
        int rating = query.value(query.record().indexOf("rating")).toInt();
        int samplerate = query.value(query.record().indexOf("samplerate")).toInt();
        int cuepoint = query.value(query.record().indexOf("cuepoint")).toInt();
        QString bpm = query.value(query.record().indexOf("bpm")).toString();
        QString replaygain = query.value(query.record().indexOf("replaygain")).toString();
        int timesplayed = query.value(query.record().indexOf("timesplayed")).toInt();
        int played = query.value(query.record().indexOf("played")).toInt();
        int channels = query.value(query.record().indexOf("channels")).toInt();
        //int filesize = query.value(query.record().indexOf("filesize")).toInt();
        QString filetype = query.value(query.record().indexOf("filetype")).toString();
        QString location = query.value(query.record().indexOf("location")).toString();
        bool header_parsed = query.value(query.record().indexOf("header_parsed")).toBool();
        QDateTime date_created = query.value(query.record().indexOf("datetime_added")).toDateTime();
        bool has_bpm_lock = query.value(query.record().indexOf("bpm_lock")).toBool();

       TrackPointer pTrack = TrackPointer(new TrackInfoObject(location, false), &TrackDAO::deleteTrack);

        // TIO already stats the file to see if it exists, what its length is,
        // etc. So don't bother setting it.
        //track->setLength(filesize);

        pTrack->setId(trackId);
        pTrack->setArtist(artist);
        pTrack->setTitle(title);
        pTrack->setAlbum(album);
        pTrack->setYear(year);
        pTrack->setGenre(genre);
        pTrack->setComposer(composer);
        pTrack->setTrackNumber(tracknumber);
        pTrack->setRating(rating);
        pTrack->setKey(key);

        pTrack->setComment(comment);
        pTrack->setURL(url);
        pTrack->setDuration(duration);
        pTrack->setBitrate(bitrate);
        pTrack->setSampleRate(samplerate);
        pTrack->setCuePoint((float)cuepoint);
        pTrack->setReplayGain(replaygain.toFloat());

        QString beatsVersion = query.value(query.record().indexOf("beats_version")).toString();
        QString beatsSubVersion = query.value(query.record().indexOf("beats_sub_version")).toString();
        QByteArray beatsBlob = query.value(query.record().indexOf("beats")).toByteArray();
        BeatsPointer pBeats = BeatFactory::loadBeatsFromByteArray(pTrack, beatsVersion, beatsSubVersion, &beatsBlob);
        if (pBeats) {
            pTrack->setBeats(pBeats);
        } else {
            pTrack->setBpm(bpm.toFloat());
        }
        pTrack->setBpmLock(has_bpm_lock);

        pTrack->setTimesPlayed(timesplayed);
        pTrack->setPlayed(played);
        pTrack->setChannels(channels);
        pTrack->setType(filetype);
        pTrack->setLocation(location);
        pTrack->setHeaderParsed(header_parsed);
        pTrack->setDateAdded(date_created);
        pTrack->setCuePoints(m_cueDao.getCuesForTrack(trackId));

        pTrack->setDirty(false);

        // Listen to dirty and changed signals
        connect(pTrack.data(), SIGNAL(dirty(TrackInfoObject*)),
                this, SLOT(slotTrackDirty(TrackInfoObject*)),
                Qt::DirectConnection);
        connect(pTrack.data(), SIGNAL(clean(TrackInfoObject*)),
                this, SLOT(slotTrackClean(TrackInfoObject*)),
                Qt::DirectConnection);
        connect(pTrack.data(), SIGNAL(changed(TrackInfoObject*)),
                this, SLOT(slotTrackChanged(TrackInfoObject*)),
                Qt::DirectConnection);
        connect(pTrack.data(), SIGNAL(save(TrackInfoObject*)),
                this, SLOT(slotTrackSave(TrackInfoObject*)),
                Qt::DirectConnection);

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

    return TrackPointer();
}

TrackPointer TrackDAO::getTrack(int id, bool cacheOnly) const {
    //qDebug() << "TrackDAO::getTrack" << QThread::currentThread() << m_database.connectionName();

    // If the track cache contains the track, use it to get a strong reference
    // to the track. We do this first so that the QCache keeps track of the
    // least-recently-used track so that it expires them intelligently.
    if (m_trackCache.contains(id)) {
        TrackPointer pTrack = *m_trackCache[id];

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

    // The person only wanted the track if it was cached.
    if (cacheOnly) {
        //qDebug() << "TrackDAO::getTrack()" << id << "Caller wanted track but only if it was cached. Returning null.";
        return TrackPointer();
    }

    QTime time;
    time.start();
    QSqlQuery query(m_database);

    query.prepare(
        "SELECT library.id, artist, title, album, year, genre, composer, tracknumber, "
        "filetype, rating, key, track_locations.location as location, "
        "track_locations.filesize as filesize, comment, url, duration, bitrate, "
        "samplerate, cuepoint, bpm, replaygain, channels, "
        "header_parsed, timesplayed, played, beats_version, beats_sub_version, beats, datetime_added, bpm_lock "
        "FROM Library "
        "INNER JOIN track_locations "
            "ON library.location = track_locations.id "
        "WHERE library.id=:track_id");
    query.bindValue(":track_id", id);

    TrackPointer pTrack;

    if (query.exec()) {
         pTrack = getTrackFromDB(query);
    } else {
        LOG_FAILED_QUERY(query)
                << QString("getTrack(%1)").arg(id);
    }
    //qDebug() << "getTrack hit the database, took " << time.elapsed() << "ms";

    return pTrack;
}

/** Saves a track's info back to the database */
void TrackDAO::updateTrack(TrackInfoObject* pTrack) {
    ScopedTransaction transaction(m_database);
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
                  "composer=:composer, filetype=:filetype, tracknumber=:tracknumber, "
                  "comment=:comment, url=:url, duration=:duration, rating=:rating, key=:key, "
                  "bitrate=:bitrate, samplerate=:samplerate, cuepoint=:cuepoint, "
                  "bpm=:bpm, replaygain=:replaygain, "
                  "timesplayed=:timesplayed, played=:played, "
                  "channels=:channels, header_parsed=:header_parsed, "
                  "beats_version=:beats_version, beats_sub_version=:beats_sub_version, beats=:beats, "
                  "bpm_lock=:bpm_lock "
                  "WHERE id=:track_id");
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":composer", pTrack->getComposer());
    query.bindValue(":filetype", pTrack->getType());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());

    query.bindValue(":replaygain", pTrack->getReplayGain());
    query.bindValue(":key", pTrack->getKey());
    query.bindValue(":rating", pTrack->getRating());
    query.bindValue(":timesplayed", pTrack->getTimesPlayed());
    query.bindValue(":played", pTrack->getPlayed());
    query.bindValue(":channels", pTrack->getChannels());
    query.bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);
    //query.bindValue(":location", pTrack->getLocation());
    query.bindValue(":track_id", trackId);

    query.bindValue(":bpm_lock", pTrack->hasBpmLock() ? 1 : 0);

    BeatsPointer pBeats = pTrack->getBeats();
    QByteArray* pBeatsBlob = NULL;
    QString beatsVersion = "";
    QString beatsSubVersion = "";
    double dBpm = pTrack->getBpm();

    if (pBeats) {
        pBeatsBlob = pBeats->toByteArray();
        beatsVersion = pBeats->getVersion();
        beatsSubVersion = pBeats->getSubVersion();
        dBpm = pBeats->getBpm();
    }
    query.bindValue(":beats", pBeatsBlob ? *pBeatsBlob : QVariant(QVariant::ByteArray));
    query.bindValue(":beats_version", beatsVersion);
    query.bindValue(":beats_sub_version", beatsSubVersion);
    query.bindValue(":bpm", dBpm);
    delete pBeatsBlob;

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "updateTrack had no effect: trackId" << trackId << "invalid";
        return;
    }

    //qDebug() << "Update track took : " << time.elapsed() << "ms. Now updating cues";
    time.start();
    m_analysisDao.saveTrackAnalyses(pTrack);
    m_cueDao.saveTrackCues(trackId, pTrack);
    transaction.commit();

    //qDebug() << "Update track in database took: " << time.elapsed() << "ms";
    time.start();
    pTrack->setDirty(false);
    //qDebug() << "Dirtying track took: " << time.elapsed() << "ms";
}

/** Mark all the tracks whose paths begin with libraryPath as invalid.
    That means we'll need to later check that those tracks actually
    (still) exist as part of the library scanning procedure. */
void TrackDAO::invalidateTrackLocationsInLibrary(QString libraryPath) {
    //qDebug() << "TrackDAO::invalidateTrackLocations" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "invalidateTrackLocations(" << libraryPath << ")";
    libraryPath += "%"; //Add wildcard to SQL query to match subdirectories!

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET needs_verification=1 "
                  "WHERE directory LIKE :directory");
    query.bindValue(":directory", libraryPath);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in directory" << libraryPath
                <<  "as needing verification.";
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
        LOG_FAILED_QUERY(query)
                << "Couldn't mark track" << location << " as verified.";
    }
}

void TrackDAO::markTracksInDirectoriesAsVerified(QStringList directories) {
    //qDebug() << "TrackDAO::markTracksInDirectoryAsVerified" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "markTracksInDirectoryAsVerified()" << directory;

    FieldEscaper escaper(m_database);
    QMutableStringListIterator it(directories);
    while (it.hasNext()) {
        it.setValue(escaper.escapeString(it.next()));
    }

    QSqlQuery query(m_database);
    query.prepare(
        QString("UPDATE track_locations "
                "SET needs_verification=0 "
                "WHERE directory IN (%1)").arg(directories.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directories.size() << "directories as verified.";
    }
}

void TrackDAO::markUnverifiedTracksAsDeleted() {
    //qDebug() << "TrackDAO::markUnverifiedTracksAsDeleted" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "markUnverifiedTracksAsDeleted()";

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1, needs_verification=0 "
                  "WHERE needs_verification=1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark unverified tracks as deleted.";
    }

}

void TrackDAO::markTrackLocationsAsDeleted(QString directory) {
    //qDebug() << "TrackDAO::markTrackLocationsAsDeleted" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directory << "as deleted.";
    }
}

/** Look for moved files. Look for files that have been marked as "deleted on disk"
    and see if another "file" with the same name and filesize exists in the track_locations
    table. That means the file has moved instead of being deleted outright, and so
    we can salvage your existing metadata that you have in your DB (like cue points, etc.). */
void TrackDAO::detectMovedFiles() {
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

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

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

void TrackDAO::clearCache() {
    m_trackCache.clear();
    m_dirtyTracks.clear();
}

void TrackDAO::writeAudioMetaData(TrackInfoObject* pTrack){
    if (m_pConfig && m_pConfig->getValueString(ConfigKey("[Library]","WriteAudioTags")).toInt() == 1) {
        AudioTagger tagger(pTrack->getLocation());

        tagger.setArtist(pTrack->getArtist());
        tagger.setTitle(pTrack->getTitle());
        tagger.setGenre(pTrack->getGenre());
        tagger.setComposer(pTrack->getComposer());
        tagger.setAlbum(pTrack->getAlbum());
        tagger.setComment(pTrack->getComment());
        tagger.setTracknumber(pTrack->getTrackNumber());
        tagger.setBpm(pTrack->getBpmStr());

        tagger.save();
    }
}

bool TrackDAO::isTrackFormatSupported(TrackInfoObject* pTrack) const {
    return SoundSourceProxy::isFilenameSupported(pTrack->getFilename());
}
