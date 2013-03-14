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
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"

QHash<int, TrackWeakPointer> TrackDAO::m_sTracks;
QMutex TrackDAO::m_sTracksMutex;

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
          m_trackCache(TRACK_CACHE_SIZE),
          m_pQueryTrackLocationInsert(NULL),
          m_pQueryTrackLocationSelect(NULL),
          m_pQueryLibraryInsert(NULL),
          m_pQueryLibraryUpdate(NULL),
          m_pQueryLibrarySelect(NULL),
          m_pTransaction(NULL) {
}

TrackDAO::~TrackDAO() {
    qDebug() << "~TrackDAO()";
    //clear all leftover Transactions
    addTracksFinish();
}

void TrackDAO::finish() {
    // scope this critical code, if this is not scoped there is a chance
    // to run in a lock of mixxx during shutdown - kain88 (July 2012)
    {
        // Save all tracks that haven't been saved yet.
        QMutexLocker locker(&m_sTracksMutex);
        QHashIterator<int, TrackWeakPointer> it(m_sTracks);
        while (it.hasNext()) {
            it.next();
            // Auto-cast from TrackWeakPointer to TrackPointer
            TrackPointer pTrack = it.value();
            if (pTrack && pTrack->isDirty()) {
                saveTrack(pTrack);
                // now pTrack->isDirty will return false
            }
        }
    }

    // Clear cache, so all cached tracks without other references where deleted
    clearCache();

    //clear out played information on exit
    //crash prevention: if mixxx crashes, played information will be maintained
    qDebug() << "Clearing played information for this session";
    QSqlQuery query(m_database);
    if (!query.exec("UPDATE library SET played=0")) {
        LOG_FAILED_QUERY(query)
                << "Error clearing played value";
    }
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

QList<int> TrackDAO::getTrackIds(QList<QFileInfo> files) {
    QStringList pathList;
    FieldEscaper escaper(m_database);
    foreach (QFileInfo file, files) {
        pathList << escaper.escapeString(file.absoluteFilePath());
    }
    QSqlQuery query(m_database);
    query.prepare(QString("SELECT library.id FROM library INNER JOIN "
                          "track_locations ON library.location = track_locations.id "
                          "WHERE track_locations.location in (%1)").arg(pathList.join(",")));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    QList<int> ids;
    while (query.next()) {
        ids.append(query.value(query.record().indexOf("id")).toInt());
    }

    return ids;
}

// Some code (eg. drag and drop) needs to just get a track's location, and it's
// not worth retrieving a whole TrackInfoObject.
QString TrackDAO::getTrackLocation(int trackId) {
    qDebug() << "TrackDAO::getTrackLocation"
             << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    QString trackLocation = "";
    query.prepare("SELECT track_locations.location FROM track_locations "
                  "INNER JOIN library ON library.location = track_locations.id "
                  "WHERE library.id=:id");
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
        return;
    }
    //qDebug() << "TrackDAO::saveTrack" << pTrack->getId() << pTrack->getInfo();
    // If track's id is not -1, then update, otherwise add.
    int trackId = pTrack->getId();
    if (trackId != -1) {
        if (pTrack->isDirty()) {
            //qDebug() << this << "Dirty tracks before clean save:" << m_dirtyTracks.size();
            //qDebug() << "TrackDAO::saveTrack. Dirty. Calling update";
            updateTrack(pTrack);

            // Write audio meta data, if enabled in the preferences
            // TODO(DSC) Only wite tag if file Metatdate is dirty
            writeAudioMetaData(pTrack);

            //qDebug() << this << "Dirty tracks remaining after clean save:" << m_dirtyTracks.size();
        } else {
            //qDebug() << "TrackDAO::saveTrack. Not Dirty";
            //qDebug() << this << "Dirty tracks remaining:" << m_dirtyTracks.size();
            //qDebug() << "Skipping track update for track" << pTrack->getId();
        }
    } else {
        addTrack(pTrack, false);
    }
}

void TrackDAO::slotTrackDirty(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackDirty" << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to complete.
    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
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
    // will wait for this slot to complete.

    if (pTrack) {
        int id = pTrack->getId();
        if (id != -1) {
            emit(trackClean(id));
        }
    }
}

void TrackDAO::databaseTrackAdded(TrackPointer pTrack) {
    emit(dbTrackAdded(pTrack));
}

void TrackDAO::databaseTracksMoved(QSet<int> tracksMovedSetOld, QSet<int> tracksMovedSetNew) {
    emit(tracksRemoved(tracksMovedSetNew));
    emit(tracksAdded(tracksMovedSetOld));  // results in a call of BaseTrackCache::updateTracksInIndex(trackIds);
}

void TrackDAO::slotTrackChanged(TrackInfoObject* pTrack) {
    //qDebug() << "TrackDAO::slotTrackChanged" << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to ask that it be saved. The only
    // time this could be unsafe is when the TIO's reference count drops to
    // 0. When that happens, the TIO is deleted with QObject:deleteLater, so Qt
    // will wait for this slot to complete.
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
    // Qt will wait for this slot to complete.
    if (pTrack) {
        saveTrack(pTrack);
    }
}

// No need to check here if the querys exist, this is already done in
// addTracksAdd, which is the only function that calls this
void TrackDAO::bindTrackToTrackLocationsInsert(TrackInfoObject* pTrack) {
    // gets called only in addTracksAdd
    m_pQueryTrackLocationInsert->bindValue(":location", pTrack->getLocation());
    m_pQueryTrackLocationInsert->bindValue(":directory", pTrack->getDirectory());
    m_pQueryTrackLocationInsert->bindValue(":filename", pTrack->getFilename());
    m_pQueryTrackLocationInsert->bindValue(":filesize", pTrack->getLength());
    // Should this check pTrack->exists()?
    m_pQueryTrackLocationInsert->bindValue(":fs_deleted", 0);
    m_pQueryTrackLocationInsert->bindValue(":needs_verification", 0);
}

// No need to check here if the querys exist, this is already done in
// addTracksAdd, which is the only function that calls this
void TrackDAO::bindTrackToLibraryInsert(TrackInfoObject* pTrack, int trackLocationId) {
    // gets called only in addTracksAdd
    m_pQueryLibraryInsert->bindValue(":artist", pTrack->getArtist());
    m_pQueryLibraryInsert->bindValue(":title", pTrack->getTitle());
    m_pQueryLibraryInsert->bindValue(":album", pTrack->getAlbum());
    m_pQueryLibraryInsert->bindValue(":year", pTrack->getYear());
    m_pQueryLibraryInsert->bindValue(":genre", pTrack->getGenre());
    m_pQueryLibraryInsert->bindValue(":composer", pTrack->getComposer());
    m_pQueryLibraryInsert->bindValue(":tracknumber", pTrack->getTrackNumber());
    m_pQueryLibraryInsert->bindValue(":filetype", pTrack->getType());
    m_pQueryLibraryInsert->bindValue(":location", trackLocationId);
    m_pQueryLibraryInsert->bindValue(":comment", pTrack->getComment());
    m_pQueryLibraryInsert->bindValue(":url", pTrack->getURL());
    m_pQueryLibraryInsert->bindValue(":duration", pTrack->getDuration());
    m_pQueryLibraryInsert->bindValue(":rating", pTrack->getRating());
    m_pQueryLibraryInsert->bindValue(":bitrate", pTrack->getBitrate());
    m_pQueryLibraryInsert->bindValue(":samplerate", pTrack->getSampleRate());
    m_pQueryLibraryInsert->bindValue(":cuepoint", pTrack->getCuePoint());
    m_pQueryLibraryInsert->bindValue(":bpm_lock", pTrack->hasBpmLock()? 1 : 0);

    m_pQueryLibraryInsert->bindValue(":replaygain", pTrack->getReplayGain());
    m_pQueryLibraryInsert->bindValue(":key", pTrack->getKey());

    // We no longer store the wavesummary in the library table.
    m_pQueryLibraryInsert->bindValue(":wavesummaryhex", QVariant(QVariant::ByteArray));

    m_pQueryLibraryInsert->bindValue(":timesplayed", pTrack->getTimesPlayed());
    //query.bindValue(":datetime_added", pTrack->getDateAdded());
    m_pQueryLibraryInsert->bindValue(":channels", pTrack->getChannels());
    m_pQueryLibraryInsert->bindValue(":mixxx_deleted", 0);
    m_pQueryLibraryInsert->bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);

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

    m_pQueryLibraryInsert->bindValue(":bpm", dBpm);
    m_pQueryLibraryInsert->bindValue(":beats_version", blobVersion);
    m_pQueryLibraryInsert->bindValue(":beats", pBeatsBlob ? *pBeatsBlob : QVariant(QVariant::ByteArray));
    delete pBeatsBlob;
}

void TrackDAO::addTracksPrepare() {

    if (m_pQueryLibraryInsert || m_pQueryTrackLocationInsert ||
        m_pQueryLibrarySelect || m_pQueryTrackLocationSelect ||
        m_pTransaction) {
        qDebug() << "TrackDAO::addTracksPrepare: old Querys have been "
                    "left open, closing them now";
        addTracksFinish();
    }
    // Start the transaction
    m_pTransaction = new ScopedTransaction(m_database);

    m_pQueryTrackLocationInsert = new QSqlQuery(m_database);
    m_pQueryTrackLocationSelect = new QSqlQuery(m_database);
    m_pQueryLibraryInsert = new QSqlQuery(m_database);
    m_pQueryLibraryUpdate = new QSqlQuery(m_database);
    m_pQueryLibrarySelect = new QSqlQuery(m_database);

    m_pQueryTrackLocationInsert->prepare("INSERT INTO track_locations "
            "(location, directory, filename, filesize, fs_deleted, needs_verification) "
            "VALUES (:location, :directory, :filename, :filesize, :fs_deleted, :needs_verification)"
            );

    m_pQueryTrackLocationSelect->prepare("SELECT id FROM track_locations WHERE location=:location");

    m_pQueryLibraryInsert->prepare("INSERT INTO library "
            "(artist, title, album, year, genre, tracknumber, "
            "filetype, location, comment, url, duration, rating, key, "
            "bitrate, samplerate, cuepoint, bpm, replaygain, wavesummaryhex, "
            "timesplayed, "
            "channels, mixxx_deleted, header_parsed, beats_version, beats_sub_version, beats, bpm_lock) "
            "VALUES ("
            ":artist, :title, :album, :year, :genre, :tracknumber, "
            ":filetype, :location, :comment, :url, :duration, :rating, :key, "
            ":bitrate, :samplerate, :cuepoint, :bpm, :replaygain, :wavesummaryhex, "
            ":timesplayed, "
            ":channels, :mixxx_deleted, :header_parsed, :beats_version, :beats_sub_version, :beats, :bpm_lock)");

    m_pQueryLibraryUpdate->prepare("UPDATE library SET mixxx_deleted = 0 "
            "WHERE id = :id");

    m_pQueryLibrarySelect->prepare("SELECT location, id, mixxx_deleted from library "
            "WHERE location = :location");
}

void TrackDAO::addTracksFinish() {
    if (m_pTransaction) {
        m_pTransaction->commit();
    }
    delete m_pQueryTrackLocationInsert;
    delete m_pQueryTrackLocationSelect;
    delete m_pQueryLibraryInsert;
    delete m_pQueryLibrarySelect;
    delete m_pTransaction;
    m_pQueryTrackLocationInsert = NULL;
    m_pQueryTrackLocationSelect = NULL;
    m_pQueryLibraryInsert = NULL;
    m_pQueryLibrarySelect = NULL;
    m_pTransaction = NULL;

    emit(tracksAdded(m_tracksAddedSet));
    m_tracksAddedSet.clear();
}

bool TrackDAO::addTracksAdd(TrackInfoObject* pTrack, bool unremove) {

    if (!m_pQueryLibraryInsert || !m_pQueryTrackLocationInsert ||
        !m_pQueryLibrarySelect || !m_pQueryTrackLocationSelect) {
        qDebug() << "TrackDAO::addTracksAdd: needed SqlQuerys have not "
                    "been prepared. Adding no tracks";
        return false;
    }

    int trackLocationId = -1;
    int trackId = -1;

    // Insert the track location into the corresponding table. This will fail
    // silently if the location is already in the table because it has a UNIQUE
    // constraint.
    bindTrackToTrackLocationsInsert(pTrack);

    if (!m_pQueryTrackLocationInsert->exec()) {
        qDebug() << "Location " << pTrack->getLocation() << " is already in the DB";
        // Inserting into track_locations failed, so the file already
        // exists. Query for its trackLocationId.

        m_pQueryTrackLocationSelect->bindValue(":location", pTrack->getLocation());

        if (!m_pQueryTrackLocationSelect->exec()) {
            // We can't even select this, something is wrong.
            LOG_FAILED_QUERY(*m_pQueryTrackLocationSelect)
                        << "Can't find track location ID after failing to insert. Something is wrong.";
            return false;
        }
        while (m_pQueryTrackLocationSelect->next()) {
            trackLocationId = m_pQueryTrackLocationSelect->value(m_pQueryTrackLocationSelect->record().indexOf("id")).toInt();
        }

        m_pQueryLibrarySelect->bindValue(":location", trackLocationId);
        if (!m_pQueryLibrarySelect->exec()) {
             LOG_FAILED_QUERY(*m_pQueryLibrarySelect)
                     << "Failed to query existing track: "
                     << pTrack->getFilename();
        } else {
            bool mixxx_deleted = 0;

            while (m_pQueryLibrarySelect->next()) {
                trackId = m_pQueryLibrarySelect->value(m_pQueryLibrarySelect->record().indexOf("id")).toInt();
                mixxx_deleted = m_pQueryLibrarySelect->value(m_pQueryLibrarySelect->record().indexOf("mixxx_deleted")).toBool();
            }
            if (unremove && mixxx_deleted) {
                // Set mixxx_deleted back to 0
                m_pQueryLibraryUpdate->bindValue(":id", trackId);
                if (!m_pQueryLibraryUpdate->exec()) {
                    LOG_FAILED_QUERY(*m_pQueryLibraryUpdate)
                            << "Failed to unremove existing track: "
                            << pTrack->getFilename();
                }
            }
            // Regardless of whether we unremoved this track or not -- it's
            // already in the library and so we need to skip it. Set the track's
            // trackId so the caller can know it. TODO(XXX) this is a little
            // weird because the track has whatever metadata the caller supplied
            // and that metadata may differ from what is already in the
            // database. I'm ignoring this corner case. rryan 10/2011
            pTrack->setId(trackId);
        }
        return false;
    } else {
        // Inserting succeeded, so just get the last rowid.
        QVariant lastInsert = m_pQueryTrackLocationInsert->lastInsertId();
        trackLocationId = lastInsert.toInt();

        //Failure of this assert indicates that we were unable to insert the track
        //location into the table AND we could not retrieve the id of that track
        //location from the same table. "It shouldn't happen"... unless I screwed up
        //- Albert :)
        Q_ASSERT(trackLocationId >= 0);

        bindTrackToLibraryInsert(pTrack, trackLocationId);

        if (!m_pQueryLibraryInsert->exec()) {
            // We failed to insert the track. Maybe it is already in the library
            // but marked deleted? Skip this track.
            LOG_FAILED_QUERY(*m_pQueryLibraryInsert)
                    << "Failed to INSERT new track into library:"
                    << pTrack->getFilename();
            return false;
        }
        trackId = m_pQueryLibraryInsert->lastInsertId().toInt();
        pTrack->setId(trackId);
        m_analysisDao.saveTrackAnalyses(pTrack);
        m_cueDao.saveTrackCues(trackId, pTrack);
        pTrack->setDirty(false);
    }
    m_tracksAddedSet.insert(trackId);
    return true;
}

int TrackDAO::addTrack(const QFileInfo& fileInfo, bool unremove) {
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

int TrackDAO::addTrack(const QString& file, bool unremove) {
    QFileInfo fileInfo(file);
    return addTrack(fileInfo, unremove);
}

void TrackDAO::addTrack(TrackInfoObject* pTrack, bool unremove) {
    //qDebug() << "TrackDAO::addTrack" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "TrackCollection::addTrack(), inserting into DB";
    Q_ASSERT(pTrack); //Why you be giving me NULL pTracks

    // Check that track is a supported extension.
    if (!isTrackFormatSupported(pTrack)) {
        // TODO(XXX) provide some kind of error code on a per-track basis.
        return;
    }

    addTracksPrepare();
    addTracksAdd(pTrack, unremove);
    addTracksFinish();
}

QList<int> TrackDAO::addTracks(QList<QFileInfo> fileInfoList, bool unremove) {
    QList<int> trackIDs;
	TrackInfoObject* pTrack;

    addTracksPrepare();

    //create the list of TrackInfoObjects from the fileInfoList
    QMutableListIterator<QFileInfo> it(fileInfoList);
    while (it.hasNext()) {
        QFileInfo& info = it.next();
        pTrack = new TrackInfoObject(info);
        addTracksAdd(pTrack, unremove);
        int trackID = pTrack->getId();
        if (trackID >= 0) {
            trackIDs.append(trackID);
        }
        delete pTrack;
    }

    addTracksFinish();
    return trackIDs;
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
    if (pTrack->isDirty()) {
        pTrack->doSave();
    }

    // Delete Track from weak reference cache
    m_sTracksMutex.lock();
    m_sTracks.remove(pTrack->getId());
    m_sTracksMutex.unlock();

    // Now Qt will delete it in the event loop.
    pTrack->deleteLater();
}

TrackPointer TrackDAO::getTrackFromDB(int id) const {
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
        "WHERE library.id=" + QString("%1").arg(id)
    );

    if (query.exec()) {
        while (query.next()) {
            // Good god! Assign query.record() to a freaking variable!
            // int trackId = query.value(query.record().indexOf("id")).toInt();
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
            QDateTime datetime_added = query.value(query.record().indexOf("datetime_added")).toDateTime();
            int played = query.value(query.record().indexOf("played")).toInt();
            int channels = query.value(query.record().indexOf("channels")).toInt();
            //int filesize = query.value(query.record().indexOf("filesize")).toInt();
            QString filetype = query.value(query.record().indexOf("filetype")).toString();
            QString location = query.value(query.record().indexOf("location")).toString();
            bool header_parsed = query.value(query.record().indexOf("header_parsed")).toBool();
            bool has_bpm_lock = query.value(query.record().indexOf("bpm_lock")).toBool();

            TrackPointer pTrack = TrackPointer(new TrackInfoObject(location, false), &TrackDAO::deleteTrack);

            // TIO already stats the file to see if it exists, what its length is,
            // etc. So don't bother setting it.
            //track->setLength(filesize);

            pTrack->setId(id);
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
                pTrack->setBpm(bpm.toDouble());
            }
            pTrack->setBpmLock(has_bpm_lock);

            pTrack->setTimesPlayed(timesplayed);
            pTrack->setDateAdded(datetime_added);
            pTrack->setPlayed(played);
            pTrack->setChannels(channels);
            pTrack->setType(filetype);
            pTrack->setLocation(location);
            pTrack->setHeaderParsed(header_parsed);
            pTrack->setCuePoints(m_cueDao.getCuesForTrack(id));

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


            m_sTracksMutex.lock();
            // Automatic conversion to a weak pointer
            m_sTracks[id] = pTrack;
            qDebug() << "m_sTracks.count() =" << m_sTracks.count();
            m_sTracksMutex.unlock();
            m_trackCache.insert(id, new TrackPointer(pTrack));

            // If the header hasn't been parsed, parse it but only after we set the
            // track clean and hooked it up to the track cache, because this will
            // dirty it.
            if (!header_parsed) {
                pTrack->parse();
            }

            return pTrack;
        } // while (query.next())

    } else {
        LOG_FAILED_QUERY(query)
            << QString("getTrack(%1)").arg(id);
    }
    //qDebug() << "getTrack hit the database, took " << time.elapsed() << "ms";

    return TrackPointer();
}

TrackPointer TrackDAO::getTrack(int id, bool cacheOnly) const {
    //qDebug() << "TrackDAO::getTrack" << QThread::currentThread() << m_database.connectionName();
    TrackPointer pTrack;

    // If the track cache contains the track, use it to get a strong reference
    // to the track. We do this first so that the QCache keeps track of the
    // least-recently-used track so that it expires them intelligently.
    if (m_trackCache.contains(id)) {
        pTrack = *m_trackCache[id];

        // If the strong reference is still valid (it should be), then return it.
        if (pTrack)
            return pTrack;
    }

    // scope this critical code so that is gets automatically unlocked
    // if this is not scoped mixxx will freeze
    {
        // Next, check the weak-reference cache to see if the track was ever loaded
        // into memory. It's possible that something is currently using this track,
        // so its reference count is non-zero despite it not being present in the
        // track cache. m_tracks is a map of weak pointers to the tracks.
        QMutexLocker locker(&m_sTracksMutex);
        if (m_sTracks.contains(id)) {
            //qDebug() << "Returning cached TIO for track" << id;
            pTrack = m_sTracks[id];
        }
    }

    // If the pointer to the cached copy is still valid, return
    // it. Otherwise, re-query the DB for the track.
    if (pTrack) {
        // Add pointer to Cache again.
        // Never call insert() inside mutex to qCache, it may trigger a
        // cache delete which requires mutex as well and cause deadlock.
        m_trackCache.insert(id, new TrackPointer(pTrack));
        return pTrack;
    }
    // The person only wanted the track if it was cached.
    if (cacheOnly) {
        //qDebug() << "TrackDAO::getTrack()" << id << "Caller wanted track but only if it was cached. Returning null.";
        return TrackPointer();
    }

    return getTrackFromDB(id);
}

// Saves a track's info back to the database
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

// Mark all the tracks whose paths begin with libraryPath as invalid.
// That means we'll need to later check that those tracks actually
// (still) exist as part of the library scanning procedure.
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

// Look for moved files. Look for files that have been marked as "deleted on disk"
// and see if another "file" with the same name and filesize exists in the track_locations
// table. That means the file has moved instead of being deleted outright, and so
// we can salvage your existing metadata that you have in your DB (like cue points, etc.).
void TrackDAO::detectMovedFiles(QSet<int>* pTracksMovedSetOld, QSet<int>* pTracksMovedSetNew) {
    //This function should not start a transaction on it's own!
    //When it's called from libraryscanner.cpp, there already is a transaction
    //started!

    QSqlQuery query(m_database);
    QSqlQuery query2(m_database);
    QSqlQuery query3(m_database);
    int oldTrackLocationId = -1;
    int newTrackLocationId = -1;
    QString filename;
    int fileSize;

    query.prepare("SELECT id, filename, filesize FROM track_locations WHERE fs_deleted=1");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    query2.prepare("SELECT id FROM track_locations WHERE "
                   "fs_deleted=0 AND "
                   "filename=:filename AND "
                   "filesize=:filesize");

    //For each track that's been "deleted" on disk...
    while (query.next()) {
        newTrackLocationId = -1; //Reset this var
        oldTrackLocationId = query.value(query.record().indexOf("id")).toInt();
        filename = query.value(query.record().indexOf("filename")).toString();
        fileSize = query.value(query.record().indexOf("filesize")).toInt();

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
            query3.prepare("DELETE FROM track_locations WHERE "
                           "id=:id");
            query3.bindValue(":id", oldTrackLocationId);
            Q_ASSERT(query3.exec());

            //The library scanner will have added a new row to the Library
            //table which corresponds to the track in the new location. We need
            //to remove that so we don't end up with two rows in the library table
            //for the same track.
            query3.prepare("SELECT id FROM library WHERE "
                           "location=:location");
            query3.bindValue(":location", newTrackLocationId);
            Q_ASSERT(query3.exec());

            while (query3.next())
            {
                int newTrackId = query3.value(query3.record().indexOf("id")).toInt();
                query3.prepare("DELETE FROM library WHERE "
                               "id=:newid");
                query3.bindValue(":newid", newTrackLocationId);
                Q_ASSERT(query3.exec());

                // We collect all the new tracks the where added to BaseTrackCache as well
                pTracksMovedSetNew->insert(newTrackId);
            }

            //Update the location foreign key for the existing row in the library table
            //to point to the correct row in the track_locations table.
            query3.prepare("SELECT id FROM library WHERE "
                           "location=:location");
            query3.bindValue(":location", oldTrackLocationId);
            Q_ASSERT(query3.exec());


            while (query3.next())
            {
                int oldTrackId = query3.value(query3.record().indexOf("id")).toInt();

                query3.prepare("UPDATE library "
                               "SET location=:newloc WHERE id=:oldid");
                query3.bindValue(":newloc", newTrackLocationId);
                query3.bindValue(":oldid", oldTrackId);
                Q_ASSERT(query3.exec());

                // We collect all the old tracks that has to be updated in BaseTrackCache as well
                pTracksMovedSetOld->insert(oldTrackId);
            }
        }
    }
}

void TrackDAO::clearCache() {
    m_trackCache.clear();
    //m_dirtyTracks.clear();
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
    if (pTrack) {
        return SoundSourceProxy::isFilenameSupported(pTrack->getFilename());
    }
    return false;
}

void TrackDAO::verifyTracksOutside(const QString& libraryPath, volatile bool* pCancel) {
    // This function is called from the LibraryScanner Thread
    ScopedTransaction transaction(m_database);
    QSqlQuery query(m_database);
    QSqlQuery query2(m_database);
    QString trackLocation;

    query.setForwardOnly(true);
    query.prepare("SELECT location "
                  "FROM track_locations "
                  "WHERE directory NOT LIKE '" +
                  libraryPath +
                  "/%'"); //Add wildcard to SQL query to match subdirectories!

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    query2.prepare("UPDATE track_locations "
                  "SET fs_deleted=:fs_deleted, needs_verification=0 "
                  "WHERE location=:location");

    while (query.next()) {
        trackLocation = query.value(query.record().indexOf("location")).toString();
        query2.bindValue(":fs_deleted", (int)!QFile::exists(trackLocation));
        query2.bindValue(":location", trackLocation);
        if (!query2.exec()) {
            LOG_FAILED_QUERY(query2);
        }
        if (*pCancel) {
            break;
        }
        emit(progressVerifyTracksOutside(trackLocation));
    }
    transaction.commit();
    qDebug() << "verifyTracksOutside finished";
}

