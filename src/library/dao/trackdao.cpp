#include <QtDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QImage>
#include <QRegExp>
#include <QCoreApplication>
#include <QChar>

#include "library/dao/trackdao.h"

#include "audiotagger.h"
#include "library/queryutil.h"
#include "soundsourceproxy.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/keyfactory.h"
#include "trackinfoobject.h"
#include "library/coverart.h"
#include "library/coverartutils.h"
#include "library/dao/cratedao.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/libraryhashdao.h"
#include "library/coverartcache.h"
#include "util/assert.h"
#include "util/timer.h"
#include "util/math.h"

QHash<int, TrackWeakPointer> TrackDAO::m_sTracks;
QMutex TrackDAO::m_sTracksMutex;

enum { UndefinedRecordIndex = -2 };

TrackCacheItem::TrackCacheItem(TrackPointer pTrack)
        : m_pTrack(pTrack) {
    DEBUG_ASSERT(m_pTrack);
}

TrackCacheItem::~TrackCacheItem() {
    DEBUG_ASSERT_AND_HANDLE(!m_pTrack.isNull()) {
        return;
    }
    // qDebug() << "~TrackCacheItem" << m_pTrack << "ID"
    //          << m_pTrack->getId() << m_pTrack->getInfo();

    // Signal to TrackDAO::saveTrack(TrackPointer) to save the track if it is
    // dirty. Does not drop the strong reference to the track until saveTrack
    // completes.
    if (m_pTrack->isDirty()) {
        emit(saveTrack(m_pTrack));
    }
}

// The number of recently used tracks to cache strong references to at
// once. Once the n+1'th track is created, the TrackDAO's QCache deletes its
// TrackCacheItem which signals to TrackDAO::saveTrack(TrackPointer) to save the
// track and drop the strong reference. The recent tracks cache basically
// functions to prevent repeated getTrack() calls for the same track from
// repeatedly deserializing / serializing a track to the database since this is
// expensive.
const int kRecentTracksCacheSize = 5;

TrackDAO::TrackDAO(QSqlDatabase& database,
                   CueDAO& cueDao,
                   PlaylistDAO& playlistDao,
                   CrateDAO& crateDao,
                   AnalysisDao& analysisDao,
                   LibraryHashDAO& libraryHashDao,
                   ConfigObject<ConfigValue> * pConfig)
        : m_database(database),
          m_cueDao(cueDao),
          m_playlistDao(playlistDao),
          m_crateDao(crateDao),
          m_analysisDao(analysisDao),
          m_libraryHashDao(libraryHashDao),
          m_pConfig(pConfig),
          m_recentTracksCache(kRecentTracksCacheSize),
          m_pQueryTrackLocationInsert(NULL),
          m_pQueryTrackLocationSelect(NULL),
          m_pQueryLibraryInsert(NULL),
          m_pQueryLibraryUpdate(NULL),
          m_pQueryLibrarySelect(NULL),
          m_pTransaction(NULL),
          m_trackLocationIdColumn(UndefinedRecordIndex),
          m_queryLibraryIdColumn(UndefinedRecordIndex),
          m_queryLibraryMixxxDeletedColumn(UndefinedRecordIndex) {
}

TrackDAO::~TrackDAO() {
    qDebug() << "~TrackDAO()";
    //clear all leftover Transactions and rollback the db
    addTracksFinish(true);
}

void TrackDAO::finish() {
    // Save all tracks that haven't been saved yet.
    QMutexLocker locker(&m_sTracksMutex);
    QHashIterator<int, TrackWeakPointer> it(m_sTracks);
    while (it.hasNext()) {
        it.next();
        // Cast from TrackWeakPointer to TrackPointer. If the track still exists
        // then pTrack will be non-NULL. If the track is dirty then save it.
        TrackPointer pTrack = it.value();
        if (!pTrack.isNull()) {
            if (pTrack->isDirty()) {
                saveTrack(pTrack);
            }

            // When this reference expires, tell the track to delete itself
            // rather than signalling to TrackDAO.
            pTrack->setDeleteOnReferenceExpiration(true);
        }
    }
    m_sTracks.clear();
    locker.unlock();

    // Clear cache, so all cached tracks without other references are deleted.
    // Will queue a bunch of saveTrack calls for every TrackCacheItem in
    // m_recentTracksCache. The event loop is already stopped at this point so
    // the queued signals won't be processed.
    clearCache();

    // Clearing the cache should queue a bunch of save events for tracks still
    // in the recent tracks cache. Deliver those events manually since we don't
    // have an event loop running anymore.
    QCoreApplication::sendPostedEvents(this, 0);

    // clear out played information on exit
    // crash prevention: if mixxx crashes, played information will be maintained
    qDebug() << "Clearing played information for this session";
    QSqlQuery query(m_database);
    if (!query.exec("UPDATE library SET played=0")) {
        LOG_FAILED_QUERY(query)
                << "Error clearing played value";
    }

    // Do housekeeping on the LibraryHashes/track_locations tables.
    qDebug() << "Cleaning LibraryHashes/track_locations tables.";
    ScopedTransaction transaction(m_database);
    QStringList deletedHashDirs = m_libraryHashDao.getDeletedDirectories();

    // Delete any LibraryHashes directories that have been marked as deleted.
    m_libraryHashDao.removeDeletedDirectoryHashes();

    // And mark the corresponding tracks in track_locations in the deleted
    // directories as deleted.
    // TODO(XXX) This doesn't handle sub-directories of deleted directories.
    foreach (QString dir, deletedHashDirs) {
        markTrackLocationsAsDeleted(dir);
    }
    transaction.commit();
}

void TrackDAO::initialize() {
    qDebug() << "TrackDAO::initialize" << QThread::currentThread() << m_database.connectionName();
}

/** Retrieve the track id for the track that's located at "location" on disk.
    @return the track id for the track located at location, or -1 if the track
            is not in the database.
*/
int TrackDAO::getTrackId(const QString& absoluteFilePath) {
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

QList<int> TrackDAO::getTrackIds(const QList<QFileInfo>& files) {
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
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        ids.append(query.value(idColumn).toInt());
    }

    return ids;
}

QSet<QString> TrackDAO::getTrackLocations() {
    QSet<QString> locations;
    QSqlQuery query(m_database);
    query.prepare("SELECT track_locations.location FROM track_locations "
                  "INNER JOIN library on library.location = track_locations.id");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    int locationColumn = query.record().indexOf("location");
    while (query.next()) {
        locations.insert(query.value(locationColumn).toString());
    }
    return locations;
}

// Some code (eg. drag and drop) needs to just get a track's location, and it's
// not worth retrieving a whole TrackInfoObject.
QString TrackDAO::getTrackLocation(const int trackId) {
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
    const int locationColumn = query.record().indexOf("location");
    while (query.next()) {
        trackLocation = query.value(locationColumn).toString();
    }

    return trackLocation;
}

/** Check if a track exists in the library table already.
    @param file_location The full path to the track on disk, including the filename.
    @return true if the track is found in the library table, false otherwise.
*/
bool TrackDAO::trackExistsInDatabase(const QString& absoluteFilePath) {
    return (getTrackId(absoluteFilePath) != -1);
}

void TrackDAO::saveTrack(TrackPointer track) {
    if (track) {
        saveTrack(track.data());
    }
}

void TrackDAO::saveTrack(TrackInfoObject* pTrack) {
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
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
        //qDebug() << "TrackDAO::saveTrack. Adding track";
        addTrack(pTrack, false);
    }
}

void TrackDAO::slotTrackDirty(TrackInfoObject* pTrack) {
    // Should not be possible.
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackDirty" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it has been
    // dirtied. It is invoked via a DirectConnection so we are sure that the
    // TrackInfoObject* has not been deleted when this is invoked. The flip side
    // of this is that this method runs in whatever thread the track was dirtied
    // from.
    int id = pTrack->getId();
    if (id != -1) {
        emit(trackDirty(id));
    }
}

void TrackDAO::slotTrackClean(TrackInfoObject* pTrack) {
    // Should not be possible.
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackClean" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it has been cleaned
    // (typically after it has been saved to the database). It is invoked via a
    // DirectConnection so we are sure that the TrackInfoObject* has not been
    // deleted when this is invoked. The flip side of this is that this method
    // runs in whatever thread the track was cleaned from.
    int id = pTrack->getId();
    if (id != -1) {
        emit(trackClean(id));
    }
}

void TrackDAO::databaseTrackAdded(TrackPointer pTrack) {
    emit(dbTrackAdded(pTrack));
}

void TrackDAO::databaseTracksMoved(QSet<int> tracksMovedSetOld, QSet<int> tracksMovedSetNew) {
    emit(tracksRemoved(tracksMovedSetNew));
    // results in a call of BaseTrackCache::updateTracksInIndex(trackIds);
    emit(tracksAdded(tracksMovedSetOld));
}

void TrackDAO::databaseTracksChanged(QSet<int> tracksChanged) {
    // results in a call of BaseTrackCache::updateTracksInIndex(trackIds);
    emit(tracksAdded(tracksChanged));
}

void TrackDAO::slotTrackChanged(TrackInfoObject* pTrack) {
    // Should not be possible.
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackChanged" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it changed.  It is
    // invoked via a DirectConnection so we are sure that the TrackInfoObject*
    // has not been deleted when this is invoked. The flip side of this is that
    // this method runs in whatever thread the track was changed from.
    int id = pTrack->getId();
    if (id != -1) {
        emit(trackChanged(id));
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
    m_pQueryLibraryInsert->bindValue(":album_artist", pTrack->getAlbumArtist());
    m_pQueryLibraryInsert->bindValue(":year", pTrack->getYear());
    m_pQueryLibraryInsert->bindValue(":genre", pTrack->getGenre());
    m_pQueryLibraryInsert->bindValue(":composer", pTrack->getComposer());
    m_pQueryLibraryInsert->bindValue(":grouping", pTrack->getGrouping());
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

    // We no longer store the wavesummary in the library table.
    m_pQueryLibraryInsert->bindValue(":wavesummaryhex", QVariant(QVariant::ByteArray));

    m_pQueryLibraryInsert->bindValue(":timesplayed", pTrack->getTimesPlayed());
    //query.bindValue(":datetime_added", pTrack->getDateAdded());
    m_pQueryLibraryInsert->bindValue(":channels", pTrack->getChannels());
    m_pQueryLibraryInsert->bindValue(":mixxx_deleted", 0);
    m_pQueryLibraryInsert->bindValue(":header_parsed", pTrack->getHeaderParsed() ? 1 : 0);

    CoverInfo coverInfo = pTrack->getCoverInfo();
    m_pQueryLibraryInsert->bindValue(":coverart_source", coverInfo.source);
    m_pQueryLibraryInsert->bindValue(":coverart_type", coverInfo.type);
    m_pQueryLibraryInsert->bindValue(":coverart_location", coverInfo.coverLocation);
    m_pQueryLibraryInsert->bindValue(":coverart_hash", coverInfo.hash);

    const QByteArray* pBeatsBlob = NULL;
    QString beatsVersion = "";
    QString beatsSubVersion = "";
    BeatsPointer pBeats = pTrack->getBeats();
    // Fall back on cached BPM
    double dBpm = pTrack->getBpm();

    if (pBeats) {
        pBeatsBlob = pBeats->toByteArray();
        beatsVersion = pBeats->getVersion();
        beatsSubVersion = pBeats->getSubVersion();
        dBpm = pBeats->getBpm();
    }

    m_pQueryLibraryInsert->bindValue(":bpm", dBpm);
    m_pQueryLibraryInsert->bindValue(":beats_version", beatsVersion);
    m_pQueryLibraryInsert->bindValue(":beats_sub_version", beatsSubVersion);
    m_pQueryLibraryInsert->bindValue(":beats", pBeatsBlob ? *pBeatsBlob : QVariant(QVariant::ByteArray));
    delete pBeatsBlob;

    const Keys& keys = pTrack->getKeys();
    QByteArray* pKeysBlob = NULL;
    QString keysVersion = "";
    QString keysSubVersion = "";
    QString keyText = "";
    mixxx::track::io::key::ChromaticKey key = mixxx::track::io::key::INVALID;

    if (keys.isValid()) {
        pKeysBlob = keys.toByteArray();
        keysVersion = keys.getVersion();
        keysSubVersion = keys.getSubVersion();
        key = keys.getGlobalKey();
        // TODO(rryan): Get this logic out of TIO.
        keyText = pTrack->getKeyText();
    }

    m_pQueryLibraryInsert->bindValue(
        ":keys", pKeysBlob ? *pKeysBlob : QVariant(QVariant::ByteArray));
    m_pQueryLibraryInsert->bindValue(":keys_version", keysVersion);
    m_pQueryLibraryInsert->bindValue(":keys_sub_version", keysSubVersion);
    m_pQueryLibraryInsert->bindValue(":key", keyText);
    m_pQueryLibraryInsert->bindValue(":key_id", static_cast<int>(key));
    delete pKeysBlob;
}

void TrackDAO::addTracksPrepare() {
        if (m_pQueryLibraryInsert || m_pQueryTrackLocationInsert ||
                m_pQueryLibrarySelect || m_pQueryTrackLocationSelect ||
                m_pTransaction) {
            qDebug() << "TrackDAO::addTracksPrepare: PROGRAMMING ERROR"
                 << "old queries have been left open, rolling back.";
        // true == do a db rollback
        addTracksFinish(true);
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
            "VALUES (:location, :directory, :filename, :filesize, :fs_deleted, :needs_verification)");

    m_pQueryTrackLocationSelect->prepare("SELECT id FROM track_locations WHERE location=:location");

    m_pQueryLibraryInsert->prepare("INSERT INTO library "
            "(artist, title, album, album_artist, year, genre, tracknumber, composer, "
            "grouping, filetype, location, comment, url, duration, rating, key, key_id, "
            "bitrate, samplerate, cuepoint, bpm, replaygain, wavesummaryhex, "
            "timesplayed, channels, mixxx_deleted, header_parsed, "
            "beats_version, beats_sub_version, beats, bpm_lock, "
            "keys_version, keys_sub_version, keys, "
            "coverart_source, coverart_type, coverart_location, coverart_hash ) "
            "VALUES ("
            ":artist, :title, :album, :album_artist, :year, :genre, :tracknumber, :composer, :grouping, "
            ":filetype, :location, :comment, :url, :duration, :rating, :key, :key_id, "
            ":bitrate, :samplerate, :cuepoint, :bpm, :replaygain, :wavesummaryhex, "
            ":timesplayed, :channels, :mixxx_deleted, :header_parsed, "
            ":beats_version, :beats_sub_version, :beats, :bpm_lock, "
            ":keys_version, :keys_sub_version, :keys, "
            ":coverart_source, :coverart_type, :coverart_location, :coverart_hash "
            ")");

    m_pQueryLibraryUpdate->prepare("UPDATE library SET mixxx_deleted = 0 "
            "WHERE id = :id");

    m_pQueryLibrarySelect->prepare("SELECT location, id, mixxx_deleted from library "
            "WHERE location = :location");
}

void TrackDAO::addTracksFinish(bool rollback) {
    if (m_pTransaction) {
        if (rollback) {
            m_pTransaction->rollback();
            m_tracksAddedSet.clear();
        } else {
            m_pTransaction->commit();
        }
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
        LOG_FAILED_QUERY(*m_pQueryTrackLocationInsert)
            << "Location " << pTrack->getLocation() << " is already in the DB";
        // Inserting into track_locations failed, so the file already
        // exists. Query for its trackLocationId.

        m_pQueryTrackLocationSelect->bindValue(":location", pTrack->getLocation());

        if (!m_pQueryTrackLocationSelect->exec()) {
            // We can't even select this, something is wrong.
            LOG_FAILED_QUERY(*m_pQueryTrackLocationSelect)
                        << "Can't find track location ID after failing to insert. Something is wrong.";
            return false;
        }
        if (m_trackLocationIdColumn == UndefinedRecordIndex) {
            m_trackLocationIdColumn = m_pQueryTrackLocationSelect->record().indexOf("id");
        }
        while (m_pQueryTrackLocationSelect->next()) {
            trackLocationId = m_pQueryTrackLocationSelect->value(m_trackLocationIdColumn).toInt();
        }

        m_pQueryLibrarySelect->bindValue(":location", trackLocationId);
        if (!m_pQueryLibrarySelect->exec()) {
             LOG_FAILED_QUERY(*m_pQueryLibrarySelect)
                     << "Failed to query existing track: "
                     << pTrack->getFilename();
        } else {
            bool mixxx_deleted = false;
            if (m_queryLibraryIdColumn == UndefinedRecordIndex) {
                QSqlRecord queryLibraryRecord = m_pQueryLibrarySelect->record();
                m_queryLibraryIdColumn = queryLibraryRecord.indexOf("id");
                m_queryLibraryMixxxDeletedColumn =
                        queryLibraryRecord.indexOf("mixxx_deleted");
            }

            while (m_pQueryLibrarySelect->next()) {
                trackId = m_pQueryLibrarySelect->value(m_queryLibraryIdColumn).toInt();
                mixxx_deleted = m_pQueryLibrarySelect->value(m_queryLibraryMixxxDeletedColumn).toBool();
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

        // Failure of this assert indicates that we were unable to insert the
        // track location into the table AND we could not retrieve the id of
        // that track location from the same table. "It shouldn't
        // happen"... unless I screwed up - Albert :)
        DEBUG_ASSERT_AND_HANDLE(trackLocationId >= 0) {
            return false;
        }

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
    // Why you be giving me NULL pTracks
    DEBUG_ASSERT_AND_HANDLE(pTrack) {
        return;
    }

    // Check that track is a supported extension.
    if (!isTrackFormatSupported(pTrack)) {
        // TODO(XXX) provide some kind of error code on a per-track basis.
        return;
    }

    addTracksPrepare();
    addTracksAdd(pTrack, unremove);
    addTracksFinish(false);
}

QList<int> TrackDAO::addTracks(const QList<QFileInfo>& fileInfoList,
                               bool unremove) {
    QSqlQuery query(m_database);
    QList<int> trackIDs;

    // Prepare to add tracks to the database.
    // This also begins an SQL transaction.
    addTracksPrepare();

    // Create a temporary database of the paths of all the imported tracks.
    query.prepare("CREATE TEMP TABLE playlist_import "
                  "(add_index INTEGER PRIMARY KEY, location varchar (512))");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        addTracksFinish(true);
        return trackIDs;
    }

    // All all the track paths to this database.
    query.prepare("INSERT INTO playlist_import (add_index, location) "
                  "VALUES (:add_index, :location)");
    int index = 0;
    foreach (const QFileInfo& rFileInfo, fileInfoList) {
        query.bindValue(":add_index", index);
        query.bindValue(":location", rFileInfo.absoluteFilePath());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
        index++;
    }

    query.prepare("SELECT library.id FROM playlist_import, "
                  "track_locations, library WHERE library.location = track_locations.id "
                  "AND playlist_import.location = track_locations.location");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    int idColumn = query.record().indexOf("id");
    while (query.next()) {
        int trackId = query.value(idColumn).toInt();
        trackIDs.append(trackId);
    }

    // If imported-playlist tracks are to be unremoved, do that for all playlist
    // tracks that were already in the database.
    if (unremove) {
        QStringList idStringList;
        foreach (int id, trackIDs) {
            idStringList.append(QString::number(id));
        }
        query.prepare(QString("UPDATE library SET mixxx_deleted=0 "
                              "WHERE id in (%1) AND mixxx_deleted=1")
                      .arg(idStringList.join(",")));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
    }

    // Any tracks not already in the database need to be added.
    query.prepare("SELECT add_index, location FROM playlist_import "
                  "WHERE NOT EXISTS (SELECT location FROM track_locations "
                  "WHERE playlist_import.location = track_locations.location)");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    const int addIndexColumn = query.record().indexOf("add_index");
    const int locationColumn = query.record().indexOf("location");
    while (query.next()) {
        int addIndex = query.value(addIndexColumn).toInt();
        QString filePath = query.value(locationColumn).toString();
        const QFileInfo& fileInfo = fileInfoList.at(addIndex);
        TrackInfoObject* pTrack = new TrackInfoObject(fileInfo);
        addTracksAdd(pTrack, unremove);
        int trackID = pTrack->getId();
        if (trackID >= 0) {
            trackIDs.append(trackID);
        }
        delete pTrack;
    }

    // Now that we have imported any tracks that were not already in the
    // library, clear trackIDs and re-select ordering by
    // playlist_import.add_index to return the list of track ids in the order
    // that they were requested to be added.
    trackIDs.clear();
    query.prepare("SELECT library.id FROM playlist_import, "
                  "track_locations, library WHERE library.location = track_locations.id "
                  "AND playlist_import.location = track_locations.location "
                  "ORDER BY playlist_import.add_index");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    idColumn = query.record().indexOf("id");
    while (query.next()) {
        int trackId = query.value(idColumn).toInt();
        trackIDs.append(trackId);
    }

    // Drop the temporary playlist-import table.
    query.prepare("DROP TABLE IF EXISTS playlist_import");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Finish adding tracks to the database.
    addTracksFinish();

    // Return the list of track IDs added to the database.
    return trackIDs;
}

void TrackDAO::hideTracks(const QList<int>& ids) {
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

    // This signal is received by basetrackcache to remove the tracks from cache
    QSet<int> tracksRemovedSet = QSet<int>::fromList(ids);
    emit(tracksRemoved(tracksRemovedSet));
}

// If a track has been manually "hidden" from Mixxx's library by the user via
// Mixxx's interface, this lets you add it back. When a track is hidden,
// mixxx_deleted in the DB gets set to 1. This clears that, and makes it show
// up in the library views again.
// This function should get called if you drag-and-drop a file that's been
// "hidden" from Mixxx back into the library view.
void TrackDAO::unhideTracks(const QList<int>& ids) {
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

void TrackDAO::purgeTracks(const QString& dir) {
    QSqlQuery query(m_database);
    FieldEscaper escaper(m_database);
    // Capture entries that start with the directory prefix dir.
    // dir needs to end in a slash otherwise we might match other
    // directories.
    QString likeClause = escaper.escapeStringForLike(QDir(dir).absolutePath() + "/", '%') + "%";

    query.prepare(QString("SELECT library.id FROM library INNER JOIN track_locations "
                          "ON library.location = track_locations.id "
                          "WHERE track_locations.location LIKE %1 ESCAPE '%'")
                  .arg(escaper.escapeString(likeClause)));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << dir;
    }

    QList<int> trackIds;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        trackIds.append(query.value(idColumn).toInt());
    }
    purgeTracks(trackIds);
}

// Warning, purge cannot be undone check before if there is no reference to this
// track id's on other library tables
void TrackDAO::purgeTracks(const QList<int>& ids) {
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
    QSqlRecord queryRecord = query.record();
    const int locationColumn = queryRecord.indexOf("location");
    const int directoryColumn = queryRecord.indexOf("directory");
    while (query.next()) {
        QString filePath = query.value(locationColumn).toString();
        locationList << escaper.escapeString(filePath);
        QString directory = query.value(directoryColumn).toString();
        dirs << escaper.escapeString(directory);
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
    QStringList dirList = QStringList::fromSet(dirs);
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
    // notify trackmodels that they should update their cache as well.
    emit(forceModelUpdate());
}

void TrackDAO::slotTrackReferenceExpired(TrackInfoObject* pTrack) {
    // Should not be possible.
    DEBUG_ASSERT_AND_HANDLE(pTrack != NULL) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackReferenceExpired" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us once its reference count
    // has dropped to zero. This is invoked via a QueuedConnection so even
    // though the track reference count can drop to zero in any thread this
    // handler runs in the main thread.

    // Save the track if it is dirty.
    if (pTrack->isDirty()) {
        saveTrack(pTrack);
    }

    // Delete Track from weak reference cache
    m_sTracksMutex.lock();
    m_sTracks.remove(pTrack->getId());
    m_sTracksMutex.unlock();

    delete pTrack;
}

namespace {
typedef bool (*TrackPopulatorFn)(const QSqlRecord& record,
                                 const int column,
                                 TrackPointer pTrack);

bool setTrackArtist(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    pTrack->setArtist(record.value(column).toString());
    return false;
}

bool setTrackTitle(const QSqlRecord& record, const int column,
                   TrackPointer pTrack) {
    pTrack->setTitle(record.value(column).toString());
    return false;
}

bool setTrackAlbum(const QSqlRecord& record, const int column,
                   TrackPointer pTrack) {
    pTrack->setAlbum(record.value(column).toString());
    return false;
}

bool setTrackAlbumArtist(const QSqlRecord& record, const int column,
                         TrackPointer pTrack) {
    pTrack->setAlbumArtist(record.value(column).toString());
    return false;
}

bool setTrackYear(const QSqlRecord& record, const int column,
                  TrackPointer pTrack) {
    pTrack->setYear(record.value(column).toString());
    return false;
}

bool setTrackGenre(const QSqlRecord& record, const int column,
                   TrackPointer pTrack) {
    pTrack->setGenre(record.value(column).toString());
    return false;
}

bool setTrackComposer(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setComposer(record.value(column).toString());
    return false;
}

bool setTrackGrouping(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setGrouping(record.value(column).toString());
    return false;
}

bool setTrackNumber(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    pTrack->setTrackNumber(record.value(column).toString());
    return false;
}

bool setTrackComment(const QSqlRecord& record, const int column,
                     TrackPointer pTrack) {
    pTrack->setComment(record.value(column).toString());
    return false;
}

bool setTrackUrl(const QSqlRecord& record, const int column,
                 TrackPointer pTrack) {
    pTrack->setURL(record.value(column).toString());
    return false;
}

bool setTrackDuration(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setDuration(record.value(column).toInt());
    return false;
}

bool setTrackBitrate(const QSqlRecord& record, const int column,
                     TrackPointer pTrack) {
    pTrack->setBitrate(record.value(column).toInt());
    return false;
}

bool setTrackRating(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    pTrack->setRating(record.value(column).toInt());
    return false;
}

bool setTrackSampleRate(const QSqlRecord& record, const int column,
                        TrackPointer pTrack) {
    pTrack->setSampleRate(record.value(column).toInt());
    return false;
}

bool setTrackCuePoint(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setCuePoint(record.value(column).toInt());
    return false;
}

bool setTrackReplayGain(const QSqlRecord& record, const int column,
                        TrackPointer pTrack) {
    pTrack->setReplayGain(record.value(column).toDouble());
    return false;
}

bool setTrackTimesPlayed(const QSqlRecord& record, const int column,
                         TrackPointer pTrack) {
    pTrack->setTimesPlayed(record.value(column).toInt());
    return false;
}

bool setTrackPlayed(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    pTrack->setPlayed(record.value(column).toBool());
    return false;
}

bool setTrackChannels(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setChannels(record.value(column).toInt());
    return false;
}

bool setTrackDateAdded(const QSqlRecord& record, const int column,
                       TrackPointer pTrack) {
    pTrack->setDateAdded(record.value(column).toDateTime());
    return false;
}

bool setTrackFiletype(const QSqlRecord& record, const int column,
                      TrackPointer pTrack) {
    pTrack->setType(record.value(column).toString());
    return false;
}

bool setTrackHeaderParsed(const QSqlRecord& record, const int column,
                          TrackPointer pTrack) {
    pTrack->setHeaderParsed(record.value(column).toBool());
    return false;
}

bool setTrackBeats(const QSqlRecord& record, const int column,
                   TrackPointer pTrack) {
    double bpm = record.value(column).toDouble();
    QString beatsVersion = record.value(column + 1).toString();
    QString beatsSubVersion = record.value(column + 2).toString();
    QByteArray beatsBlob = record.value(column + 3).toByteArray();
    bool bpmLocked = record.value(column + 4).toBool();
    BeatsPointer pBeats = BeatFactory::loadBeatsFromByteArray(
            pTrack, beatsVersion, beatsSubVersion, &beatsBlob);
    if (pBeats) {
        pTrack->setBeats(pBeats);
    } else {
        pTrack->setBpm(bpm);
    }
    pTrack->setBpmLock(bpmLocked);
    return false;
}

bool setTrackKey(const QSqlRecord& record, const int column,
                 TrackPointer pTrack) {
    QString keyText = record.value(column).toString();
    QString keysVersion = record.value(column + 1).toString();
    QString keysSubVersion = record.value(column + 2).toString();
    QByteArray keysBlob = record.value(column + 3).toByteArray();
    Keys keys = KeyFactory::loadKeysFromByteArray(
            keysVersion, keysSubVersion, &keysBlob);

    if (keys.isValid()) {
        pTrack->setKeys(keys);
    } else if (keyText.size() > 0) {
        // Typically this happens if we are upgrading from an older (<1.12.0)
        // version of Mixxx that didn't support Keys. We treat all legacy data
        // as user-generated because that way it will be treated sensitively.
        pTrack->setKeyText(keyText, mixxx::track::io::key::USER);
        // The in-database data would change because of this. Mark the track
        // dirty so we save it when it is deleted.
        return true;
    }
    return false;
}

bool setTrackCoverInfo(const QSqlRecord& record, const int column,
                       TrackPointer pTrack) {
    CoverInfo coverInfo;
    bool ok = false;
    coverInfo.source = static_cast<CoverInfo::Source>(
            record.value(column).toInt(&ok));
    if (!ok) coverInfo.source = CoverInfo::UNKNOWN;
    coverInfo.type = static_cast<CoverInfo::Type>(
            record.value(column + 1).toInt(&ok));
    if (!ok) coverInfo.type = CoverInfo::NONE;
    coverInfo.coverLocation = record.value(column + 2).toString();
    coverInfo.hash = record.value(column + 3).toUInt();
    pTrack->setCoverInfo(coverInfo);
    return false;
}

struct ColumnPopulator {
    const char* name;
    TrackPopulatorFn populator;
};

}  // namespace

#define ARRAYLENGTH(x) (sizeof(x) / sizeof(*x))

TrackPointer TrackDAO::getTrackFromDB(const int id) const {
    ScopedTimer t("TrackDAO::getTrackFromDB");
    QSqlQuery query(m_database);

    ColumnPopulator columns[] = {
        // Location must be first.
        { "track_locations.location", NULL },
        { "artist", setTrackArtist },
        { "title", setTrackTitle },
        { "album", setTrackAlbum },
        { "album_artist", setTrackAlbumArtist },
        { "year", setTrackYear },
        { "genre", setTrackGenre },
        { "composer", setTrackComposer },
        { "grouping", setTrackGrouping },
        { "tracknumber", setTrackNumber },
        { "filetype", setTrackFiletype },
        { "rating", setTrackRating },
        { "comment", setTrackComment },
        { "url", setTrackUrl },
        { "duration", setTrackDuration },
        { "bitrate", setTrackBitrate },
        { "samplerate", setTrackSampleRate },
        { "cuepoint", setTrackCuePoint },
        { "replaygain", setTrackReplayGain },
        { "channels", setTrackChannels },
        { "timesplayed", setTrackTimesPlayed },
        { "played", setTrackPlayed },
        { "datetime_added", setTrackDateAdded },
        { "header_parsed", setTrackHeaderParsed },

        // Beat detection columns are handled by setTrackBeats. Do not change
        // the ordering of these columns or put other columns in between them!
        { "bpm", setTrackBeats },
        { "beats_version", NULL },
        { "beats_sub_version", NULL },
        { "beats", NULL },
        { "bpm_lock", NULL },

        // Beat detection columns are handled by setTrackKey. Do not change the
        // ordering of these columns or put other columns in between them!
        { "key", setTrackKey },
        { "keys_version", NULL },
        { "keys_sub_version", NULL },
        { "keys", NULL },

        // Cover art columns are handled by setTrackCoverInfo. Do not change the
        // ordering of these columns or put other columns in between them!
        { "coverart_source", setTrackCoverInfo },
        { "coverart_type", NULL },
        { "coverart_location", NULL },
        { "coverart_hash", NULL }
    };

    QString columnsStr;
    int columnsSize = 0;
    const int columnsCount = ARRAYLENGTH(columns);
    for (int i = 0; i < columnsCount; ++i) {
        columnsSize += qstrlen(columns[i].name) + 1;
    }
    columnsStr.reserve(columnsSize);
    for (int i = 0; i < columnsCount; ++i) {
        if (i > 0) {
            columnsStr.append(QChar(','));
        }
        columnsStr.append(columns[i].name);
    }

    query.prepare(QString(
            "SELECT %1 FROM Library "
            "INNER JOIN track_locations ON library.location = track_locations.id "
            "WHERE library.id = %2").arg(columnsStr, QString::number(id)));

    if (!query.exec() || !query.next()) {
        LOG_FAILED_QUERY(query)
                << QString("getTrack(%1)").arg(id);
        return TrackPointer();
    }

    QSqlRecord queryRecord = query.record();
    int recordCount = queryRecord.count();
    DEBUG_ASSERT_AND_HANDLE(recordCount == columnsCount) {
        recordCount = math_min(recordCount, columnsCount);
    }

    // Location is the first column.
    QString location = queryRecord.value(0).toString();

    TrackPointer pTrack = TrackPointer(
            new TrackInfoObject(location, SecurityTokenPointer(),
                                false),
            TrackInfoObject::onTrackReferenceExpired);
    pTrack->setId(id);

    // TIO already stats the file to see if it exists, what its length is,
    // etc. So don't bother setting it.

    // For every column run its populator to fill the track in with the data.
    bool shouldDirty = false;
    for (int i = 0; i < recordCount; ++i) {
        TrackPopulatorFn populator = columns[i].populator;
        if (populator != NULL) {
            // If any populator says the track should be dirty then we dirty it.
            shouldDirty = (*populator)(queryRecord, i, pTrack) || shouldDirty;
        }
    }

    // Populate track cues from the cues table.
    pTrack->setCuePoints(m_cueDao.getCuesForTrack(id));

    // Normally we will set the track as clean but sometimes when loading from
    // the database we need to perform upkeep that ought to be written back to
    // the database when the track is deleted.
    pTrack->setDirty(shouldDirty);

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
    // Queued connection. We are not in a rush to process reference
    // count expirations and it can produce dangerous signal loops.
    // See: https://bugs.launchpad.net/mixxx/+bug/1365708
    connect(pTrack.data(), SIGNAL(referenceExpired(TrackInfoObject*)),
            this, SLOT(slotTrackReferenceExpired(TrackInfoObject*)),
            Qt::QueuedConnection);

    m_sTracksMutex.lock();
    // Automatic conversion to a weak pointer
    m_sTracks[id] = pTrack;
    qDebug() << "m_sTracks.count() =" << m_sTracks.count();
    m_sTracksMutex.unlock();
    TrackCacheItem* pCacheItem = new TrackCacheItem(pTrack);

    // Queued connection. We are not in a rush to process cache
    // expirations and it can produce dangerous signal loops.
    // See: https://bugs.launchpad.net/mixxx/+bug/1365708
    connect(pCacheItem, SIGNAL(saveTrack(TrackPointer)),
            this, SLOT(saveTrack(TrackPointer)),
            Qt::QueuedConnection);

    m_recentTracksCache.insert(id, pCacheItem);

    // If the track is dirty send dirty notifications after we inserted
    // it in the cache. BaseTrackCache cares about dirty notifications
    // and the setDirty call above happens before we connect to the
    // track's signals.
    if (shouldDirty) {
        emit(trackDirty(id));
    }

    // If the header hasn't been parsed, parse it but only after we set the
    // track clean and hooked it up to the track cache, because this will
    // dirty it.
    if (!pTrack->getHeaderParsed()) {
         pTrack->parse(false);
    }

    return pTrack;
}

TrackPointer TrackDAO::getTrack(const int id, const bool cacheOnly) const {
    //qDebug() << "TrackDAO::getTrack" << QThread::currentThread() << m_database.connectionName();
    TrackPointer pTrack;

    // If the track cache contains the track ID, use it to get a strong
    // reference to the track. We do this first so that the QCache keeps track
    // of the least-recently-used track so that it expires them intelligently.
    TrackCacheItem* pTrackCacheItem = m_recentTracksCache.object(id);
    if (pTrackCacheItem != NULL) {
        pTrack = pTrackCacheItem->getTrack();
        // If the strong reference is still valid (it should be), then return
        // it.
        DEBUG_ASSERT(pTrack);
        if (pTrack) {
            return pTrack;
        }
    }

    // Next, check the weak-reference cache to see if the track still has a
    // strong reference somewhere. It's possible that something is currently
    // using this track so its reference count is non-zero despite it not being
    // present in the track cache.
    QMutexLocker locker(&m_sTracksMutex);
    QHash<int, TrackWeakPointer>::iterator it = m_sTracks.find(id);
    if (it != m_sTracks.end()) {
        //qDebug() << "Returning cached TIO for track" << id;
        pTrack = it.value();
    }

    // Unlock the track cache mutex. Otherwise we can deadlock.
    locker.unlock();

    // If we were able to convert a weak reference to a strong reference then
    // re-insert it into the recent tracks cache so that its least-recently-used
    // tracking is accurate.
    if (pTrack) {
        // NOTE: Never call QCache::insert() while holding the weak-reference
        // hash mutex. It may trigger a cache delete and trigger a deadlock.
        TrackCacheItem* pCacheItem = new TrackCacheItem(pTrack);

        // Queued connection. We are not in a rush to process cache
        // expirations and it can produce dangerous signal loops.
        // See: https://bugs.launchpad.net/mixxx/+bug/1365708
        connect(pCacheItem, SIGNAL(saveTrack(TrackPointer)),
                this, SLOT(saveTrack(TrackPointer)),
                Qt::QueuedConnection);

        m_recentTracksCache.insert(id, pCacheItem);
        return pTrack;
    } else if (cacheOnly) {
        // The caller only wanted the track if it was cached.
        //qDebug() << "TrackDAO::getTrack()" << id << "Caller wanted track but only if it was cached. Returning null.";
        return TrackPointer();
    }

    // Otherwise, deserialize the track from the database.
    return getTrackFromDB(id);
}

// Saves a track's info back to the database
void TrackDAO::updateTrack(TrackInfoObject* pTrack) {
    DEBUG_ASSERT_AND_HANDLE(pTrack) {
        return;
    }

    ScopedTransaction transaction(m_database);
    // QTime time;
    // time.start();
    //qDebug() << "TrackDAO::updateTrackInDatabase" << QThread::currentThread() << m_database.connectionName();

    //qDebug() << "Updating track" << pTrack->getInfo() << "in database...";

    int trackId = pTrack->getId();
    DEBUG_ASSERT_AND_HANDLE(trackId >= 0) {
        return;
    }

    QSqlQuery query(m_database);

    //Update everything but "location", since that's what we identify the track by.
    query.prepare("UPDATE library "
                  "SET artist=:artist, "
                  "title=:title, album=:album, "
                  "album_artist=:album_artist, "
                  "year=:year, genre=:genre, composer=:composer, "
                  "grouping=:grouping, filetype=:filetype, "
                  "tracknumber=:tracknumber, comment=:comment, url=:url, "
                  "duration=:duration, rating=:rating, "
                  "key=:key, key_id=:key_id, "
                  "bitrate=:bitrate, samplerate=:samplerate, cuepoint=:cuepoint, "
                  "bpm=:bpm, replaygain=:replaygain, "
                  "timesplayed=:timesplayed, played=:played, "
                  "channels=:channels, header_parsed=:header_parsed, "
                  "beats_version=:beats_version, beats_sub_version=:beats_sub_version, beats=:beats, "
                  "bpm_lock=:bpm_lock, "
                  "keys_version=:keys_version, keys_sub_version=:keys_sub_version, keys=:keys, "
                  "coverart_source=:coverart_source, coverart_type=:coverart_type, "
                  "coverart_location=:coverart_location, coverart_hash=:coverart_hash "
                  "WHERE id=:track_id");
    query.bindValue(":artist", pTrack->getArtist());
    query.bindValue(":title", pTrack->getTitle());
    query.bindValue(":album", pTrack->getAlbum());
    query.bindValue(":album_artist", pTrack->getAlbumArtist());
    query.bindValue(":year", pTrack->getYear());
    query.bindValue(":genre", pTrack->getGenre());
    query.bindValue(":composer", pTrack->getComposer());
    query.bindValue(":grouping", pTrack->getGrouping());
    query.bindValue(":filetype", pTrack->getType());
    query.bindValue(":tracknumber", pTrack->getTrackNumber());
    query.bindValue(":comment", pTrack->getComment());
    query.bindValue(":url", pTrack->getURL());
    query.bindValue(":duration", pTrack->getDuration());
    query.bindValue(":bitrate", pTrack->getBitrate());
    query.bindValue(":samplerate", pTrack->getSampleRate());
    query.bindValue(":cuepoint", pTrack->getCuePoint());

    query.bindValue(":replaygain", pTrack->getReplayGain());
    query.bindValue(":rating", pTrack->getRating());
    query.bindValue(":timesplayed", pTrack->getTimesPlayed());
    query.bindValue(":played", pTrack->getPlayed() ? 1 : 0);
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

    const Keys& keys = pTrack->getKeys();
    QByteArray* pKeysBlob = NULL;
    QString keysVersion = "";
    QString keysSubVersion = "";
    QString keyText = "";
    mixxx::track::io::key::ChromaticKey key = mixxx::track::io::key::INVALID;

    if (keys.isValid()) {
        pKeysBlob = keys.toByteArray();
        keysVersion = keys.getVersion();
        keysSubVersion = keys.getSubVersion();
        key = keys.getGlobalKey();
        // TODO(rryan): Get this logic out of TIO.
        keyText = pTrack->getKeyText();
    }

    query.bindValue(":keys", pKeysBlob ? *pKeysBlob : QVariant(QVariant::ByteArray));
    query.bindValue(":keys_version", keysVersion);
    query.bindValue(":keys_sub_version", keysSubVersion);
    query.bindValue(":key", keyText);
    query.bindValue(":key_id", static_cast<int>(key));
    delete pKeysBlob;

    CoverInfo coverInfo = pTrack->getCoverInfo();
    query.bindValue(":coverart_source", coverInfo.source);
    query.bindValue(":coverart_type", coverInfo.type);
    query.bindValue(":coverart_location", coverInfo.coverLocation);
    query.bindValue(":coverart_hash", coverInfo.hash);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "updateTrack had no effect: trackId" << trackId << "invalid";
        return;
    }

    //qDebug() << "Update track took : " << time.elapsed() << "ms. Now updating cues";
    //time.start();
    m_analysisDao.saveTrackAnalyses(pTrack);
    m_cueDao.saveTrackCues(trackId, pTrack);
    transaction.commit();

    //qDebug() << "Update track in database took: " << time.elapsed() << "ms";
    //time.start();
    pTrack->setDirty(false);
    //qDebug() << "Dirtying track took: " << time.elapsed() << "ms";
}

// Mark all the tracks in the library as invalid.
// That means we'll need to later check that those tracks actually
// (still) exist as part of the library scanning procedure.
void TrackDAO::invalidateTrackLocationsInLibrary() {
    //qDebug() << "TrackDAO::invalidateTrackLocations" << QThread::currentThread() << m_database.connectionName();
    //qDebug() << "invalidateTrackLocations(" << libraryPath << ")";

    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations SET needs_verification = 1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in library as needing verification.";
    }
}

void TrackDAO::markTrackLocationsAsVerified(const QStringList& locations) {
    //qDebug() << "TrackDAO::markTrackLocationsAsVerified" << QThread::currentThread() << m_database.connectionName();

    FieldEscaper escaper(m_database);
    QStringList escapedLocations = escaper.escapeStrings(locations);

    QSqlQuery query(m_database);
    query.prepare(QString("UPDATE track_locations "
                          "SET needs_verification=0, fs_deleted=0 "
                          "WHERE location IN (%1)").arg(
                              escapedLocations.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark track locations as verified.";
    }
}

void TrackDAO::markTracksInDirectoriesAsVerified(const QStringList& directories) {
    //qDebug() << "TrackDAO::markTracksInDirectoryAsVerified" << QThread::currentThread() << m_database.connectionName();

    FieldEscaper escaper(m_database);
    QStringList escapedDirectories = escaper.escapeStrings(directories);

    QSqlQuery query(m_database);
    query.prepare(
        QString("UPDATE track_locations "
                "SET needs_verification=0 "
                "WHERE directory IN (%1)").arg(escapedDirectories.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directories.size() << "directories as verified.";
    }
}

void TrackDAO::markUnverifiedTracksAsDeleted() {
    //qDebug() << "TrackDAO::markUnverifiedTracksAsDeleted" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT library.id as id FROM library INNER JOIN track_locations ON "
                  "track_locations.id=library.location WHERE "
                  "track_locations.needs_verification=1");
    QSet<int> trackIds;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Couldn't find unverified tracks";
    }
    while (query.next()) {
        trackIds.insert(query.value(query.record().indexOf("id")).toInt());
    }
    emit(tracksRemoved(trackIds));
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1, needs_verification=0 "
                  "WHERE needs_verification=1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark unverified tracks as deleted.";
    }
}

void TrackDAO::markTrackLocationsAsDeleted(const QString& directory) {
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
    // rather use duration then filesize as an indicator of changes. The filesize
    // can change by adding more ID3v2 tags
    int duration = -1;

    query.prepare("SELECT track_locations.id, filename, duration FROM track_locations "
                  "INNER JOIN library ON track_locations.id=library.location "
                  "WHERE fs_deleted=1");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    query2.prepare("SELECT track_locations.id FROM track_locations "
                   "INNER JOIN library ON track_locations.id=library.location "
                   "WHERE fs_deleted=0 AND "
                   "filename=:filename AND "
                   "duration=:duration");

    QSqlRecord queryRecord = query.record();
    const int idColumn = queryRecord.indexOf("id");
    const int filenameColumn = queryRecord.indexOf("filename");
    const int durationColumn = queryRecord.indexOf("duration");

    //For each track that's been "deleted" on disk...
    while (query.next()) {
        newTrackLocationId = -1; //Reset this var
        oldTrackLocationId = query.value(idColumn).toInt();
        filename = query.value(filenameColumn).toString();
        duration = query.value(durationColumn).toInt();

        query2.bindValue(":filename", filename);
        query2.bindValue(":duration", duration);
        if (!query2.exec()) {
            // Should not happen!
            LOG_FAILED_QUERY(query2);
        }
        // WTF duplicate tracks?
        if (query2.size() > 1) {
            LOG_FAILED_QUERY(query2) << "Result size was greater than 1.";
        }

        const int query2idColumn = query2.record().indexOf("id");
        while (query2.next()) {
            newTrackLocationId = query2.value(query2idColumn).toInt();
        }

        //If we found a moved track...
        if (newTrackLocationId >= 0) {
            qDebug() << "Found moved track!" << filename;

            // Remove old row from track_locations table
            query3.prepare("DELETE FROM track_locations WHERE id=:id");
            query3.bindValue(":id", oldTrackLocationId);
            if (!query3.exec()) {
                // Should not happen!
                LOG_FAILED_QUERY(query3);
            }

            // The library scanner will have added a new row to the Library
            // table which corresponds to the track in the new location. We need
            // to remove that so we don't end up with two rows in the library
            // table for the same track.
            query3.prepare("SELECT id FROM library WHERE location=:location");
            query3.bindValue(":location", newTrackLocationId);
            if (!query3.exec()) {
                // Should not happen!
                LOG_FAILED_QUERY(query3);
            }

            const int query3idColumn = query3.record().indexOf("id");
            if (query3.next()) {
                int newTrackId = query3.value(query3idColumn).toInt();
                query3.prepare("DELETE FROM library WHERE id=:newid");
                query3.bindValue(":newid", newTrackLocationId);
                if (!query3.exec()) {
                    // Should not happen!
                    LOG_FAILED_QUERY(query3);
                }
                // We collect all the new tracks the where added to BaseTrackCache as well
                pTracksMovedSetNew->insert(newTrackId);
            }
            // Delete the track
            query3.prepare("DELETE FROM library WHERE id=:newid");
            query3.bindValue(":newid", newTrackLocationId);
            if (!query3.exec()) {
                // Should not happen!
                LOG_FAILED_QUERY(query3);
            }

            // Update the location foreign key for the existing row in the
            // library table to point to the correct row in the track_locations
            // table.
            query3.prepare("SELECT id FROM library WHERE location=:location");
            query3.bindValue(":location", oldTrackLocationId);
            if (!query3.exec()) {
                // Should not happen!
                LOG_FAILED_QUERY(query3);
            }

            if (query3.next()) {
                int oldTrackId = query3.value(query3idColumn).toInt();
                query3.prepare("UPDATE library SET location=:newloc WHERE id=:oldid");
                query3.bindValue(":newloc", newTrackLocationId);
                query3.bindValue(":oldid", oldTrackId);
                if (!query3.exec()) {
                    // Should not happen!
                    LOG_FAILED_QUERY(query3);
                }

                // We collect all the old tracks that has to be updated in BaseTrackCache as well
                pTracksMovedSetOld->insert(oldTrackId);
            }
        }
    }
}

void TrackDAO::clearCache() {
    // Triggers a deletion of all the TrackCacheItems which in turn calls
    // saveTrack(TrackPointer) for all of the tracks in the recent tracks cache.
    m_recentTracksCache.clear();
}

void TrackDAO::markTracksAsMixxxDeleted(const QString& dir) {
    QSqlQuery query(m_database);

    FieldEscaper escaper(m_database);

    // Capture entries that start with the directory prefix dir.
    // dir needs to end in a slash otherwise we might match other
    // directories.
    QString likeClause = escaper.escapeStringForLike(dir + "/", '%') + "%";

    query.prepare(QString("SELECT library.id FROM library INNER JOIN track_locations "
                          "ON library.location = track_locations.id "
                          "WHERE track_locations.location LIKE %1 ESCAPE '%'")
                  .arg(escaper.escapeString(likeClause)));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << dir;
    }

    QStringList trackIds;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        trackIds.append(QString::number(query.value(idColumn).toInt()));
    }

    query.prepare(QString("UPDATE library SET mixxx_deleted=1 "
                          "WHERE id in (%1)").arg(trackIds.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

void TrackDAO::writeAudioMetaData(TrackInfoObject* pTrack) {
    if (m_pConfig && m_pConfig->getValueString(ConfigKey("[Library]","WriteAudioTags")).toInt() == 1) {
        AudioTagger tagger(pTrack->getLocation(), pTrack->getSecurityToken());

        tagger.setArtist(pTrack->getArtist());
        tagger.setTitle(pTrack->getTitle());
        tagger.setGenre(pTrack->getGenre());
        tagger.setComposer(pTrack->getComposer());
        tagger.setGrouping(pTrack->getGrouping());
        tagger.setAlbum(pTrack->getAlbum());
        tagger.setAlbumArtist(pTrack->getAlbumArtist());
        tagger.setComment(pTrack->getComment());
        tagger.setTracknumber(pTrack->getTrackNumber());
        tagger.setBpm(pTrack->getBpmStr());
        tagger.setKey(pTrack->getKeyText());
        tagger.setComposer(pTrack->getComposer());
        tagger.setGrouping(pTrack->getGrouping());

        tagger.save();
    }
}

bool TrackDAO::isTrackFormatSupported(TrackInfoObject* pTrack) const {
    if (pTrack) {
        return SoundSourceProxy::isFilenameSupported(pTrack->getFilename());
    }
    return false;
}

void TrackDAO::verifyRemainingTracks() {
    // This function is called from the LibraryScanner Thread, which also has a
    // transaction running, so we do NOT NEED to use one here
    QSqlQuery query(m_database);
    QSqlQuery query2(m_database);

    // Because all tracks were marked with needs_verification anything that is
    // not inside one of the tracked library directories will need an explicit
    // check if it exists.
    // TODO(kain88) check if all others are marked with 0 again
    query.setForwardOnly(true);
    query.prepare("SELECT location "
                  "FROM track_locations "
                  "WHERE needs_verification = 1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return;
    }

    query2.prepare("UPDATE track_locations "
                   "SET fs_deleted=:fs_deleted, needs_verification=0 "
                   "WHERE location=:location");

    const int locationColumn = query.record().indexOf("location");
    QString trackLocation;
    while (query.next()) {
        trackLocation = query.value(locationColumn).toString();
        query2.bindValue(":fs_deleted", QFile::exists(trackLocation) ? 0 : 1);
        query2.bindValue(":location", trackLocation);
        if (!query2.exec()) {
            LOG_FAILED_QUERY(query2);
        }
        emit(progressVerifyTracksOutside(trackLocation));
    }
}

namespace
{
    QImage parseCoverArt(const QFileInfo& fileInfo) {
        SecurityTokenPointer pToken = Sandbox::openSecurityToken(fileInfo, true);
        SoundSourceProxy proxy(fileInfo.filePath(), pToken);
        Mixxx::SoundSourcePointer pSoundSource(proxy.getSoundSource());
        if (pSoundSource) {
            return pSoundSource->parseCoverArt();
        } else {
            return QImage();
        }
    }
}

struct TrackWithoutCover {
    TrackWithoutCover() : trackId(-1) { }
    int trackId;
    QString trackLocation;
    QString directoryPath;
    QString trackAlbum;
};

void TrackDAO::detectCoverArtForUnknownTracks(volatile const bool* pCancel,
                                              QSet<int>* pTracksChanged) {
    // WARNING TO ANYONE TOUCHING THIS IN THE FUTURE
    // The library contains user selected cover art. There is nothing worse than
    // spending hours curating your library only to have an automated search
    // method like this one replace it all with its mistakes again. Take care to
    // not modify any tracks with coverart_source equal to USER_SELECTED (value
    // 2).

    QSqlQuery query(m_database);
    query.setForwardOnly(true);
    query.prepare("SELECT "
                  " library.id, " // 0
                  " track_locations.location, " // 1
                  " track_locations.directory, " // 2
                  " album, " // 3
                  " coverart_source " // 4
                  "FROM library "
                  "INNER JOIN track_locations "
                  "ON library.location = track_locations.id "
                  // CoverInfo::Source 0 is UNKNOWN
                  "WHERE coverart_source is NULL or coverart_source = 0 "
                  "ORDER BY track_locations.directory");

    QList<TrackWithoutCover> tracksWithoutCover;

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "failed looking for tracks with unknown cover art";
        return;
    }

    // We quickly iterate through the results to prevent blocking the database
    // for other operations. Bug #1399981.
    while (query.next()) {
        if (*pCancel) {
            return;
        }

        TrackWithoutCover track;
        track.trackId = query.value(0).toInt();
        track.trackLocation = query.value(1).toString();
        // TODO(rryan) use QFileInfo path instead? symlinks? relative?
        track.directoryPath = query.value(2).toString();
        track.trackAlbum = query.value(3).toString();

        CoverInfo::Source source = static_cast<CoverInfo::Source>(
            query.value(4).toInt());
        if (source == CoverInfo::USER_SELECTED) {
            qWarning() << "PROGRAMMING ERROR! detectCoverArtForUnknownTracks"
                       << "got a USER_SELECTED track. Skipping.";
            continue;
        }
        tracksWithoutCover.append(track);
    }

    QSqlQuery updateQuery(m_database);
    updateQuery.prepare(
        "UPDATE library SET "
        "  coverart_type=:coverart_type,"
        "  coverart_source=:coverart_source,"
        "  coverart_hash=:coverart_hash,"
        "  coverart_location=:coverart_location "
        "WHERE id=:track_id");


    QRegExp coverArtFilenames(CoverArtUtils::supportedCoverArtExtensionsRegex(),
                              Qt::CaseInsensitive);
    QString currentDirectoryPath;
    MDir currentDirectory;
    QLinkedList<QFileInfo> possibleCovers;

    foreach (const TrackWithoutCover& track, tracksWithoutCover) {
        if (*pCancel) {
            return;
        }

        //qDebug() << "Searching for cover art for" << trackLocation;
        emit(progressCoverArt(track.trackLocation));

        QFileInfo trackInfo(track.trackLocation);
        if (!trackInfo.exists()) {
            //qDebug() << trackLocation << "does not exist";
            continue;
        }

        QImage image(parseCoverArt(trackInfo));
        if (!image.isNull()) {
            updateQuery.bindValue(":coverart_type",
                                  static_cast<int>(CoverInfo::METADATA));
            updateQuery.bindValue(":coverart_source",
                                  static_cast<int>(CoverInfo::GUESSED));
            updateQuery.bindValue(":coverart_hash",
                                  CoverArtUtils::calculateHash(image));
            updateQuery.bindValue(":coverart_location", "");
            updateQuery.bindValue(":track_id", track.trackId);
            if (!updateQuery.exec()) {
                LOG_FAILED_QUERY(updateQuery) << "failed to write metadata cover";
            } else {
                pTracksChanged->insert(track.trackId);
            }
            continue;
        }

        if (track.directoryPath != currentDirectoryPath) {
            possibleCovers.clear();
            currentDirectoryPath = track.directoryPath;
            currentDirectory = MDir(currentDirectoryPath);
            possibleCovers = CoverArtUtils::findPossibleCoversInFolder(
                currentDirectoryPath);
        }

        CoverArt art = CoverArtUtils::selectCoverArtForTrack(
            trackInfo.baseName(), track.trackAlbum, possibleCovers);

        updateQuery.bindValue(":coverart_type",
                              static_cast<int>(art.info.type));
        updateQuery.bindValue(":coverart_source",
                              static_cast<int>(art.info.source));
        updateQuery.bindValue(":coverart_hash", art.info.hash);
        updateQuery.bindValue(":coverart_location", art.info.coverLocation);
        updateQuery.bindValue(":track_id", track.trackId);
        if (!updateQuery.exec()) {
            LOG_FAILED_QUERY(updateQuery) << "failed to write file or none cover";
        } else {
            pTracksChanged->insert(track.trackId);
        }
    }
}

TrackPointer TrackDAO::getOrAddTrack(const QString& trackLocation,
                                     bool processCoverArt,
                                     bool* pAlreadyInLibrary) {
    int track_id = getTrackId(trackLocation);
    bool track_already_in_library = track_id >= 0;

    // Add Track to library -- unremove if it was previously removed.
    if (track_id < 0) {
        track_id = addTrack(trackLocation, true);
    }

    TrackPointer pTrack;
    if (track_id >= 0) {
        pTrack = getTrack(track_id);
    }

    // addTrack or getTrack may fail. If they did, create a transient
    // TrackPointer. We explicitly do not process cover art while creating the
    // TrackInfoObject since we want to do it asynchronously (see below).
    if (pTrack.isNull()) {
        pTrack = TrackPointer(new TrackInfoObject(
                trackLocation, SecurityTokenPointer(), true, false));
    }

    // If the track wasn't in the library already then it has not yet been
    // checked for cover art. If processCoverArt is true then we should request
    // cover processing via CoverArtCache asynchronously.
    if (processCoverArt && pTrack && !track_already_in_library) {
        CoverArtCache* pCache = CoverArtCache::instance();
        if (pCache != NULL) {
            pCache->requestGuessCover(pTrack);
        }
    }

    if (pAlreadyInLibrary != NULL) {
        *pAlreadyInLibrary = track_already_in_library;
    }

    return pTrack;
}
