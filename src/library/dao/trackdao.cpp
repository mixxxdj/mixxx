#include "library/dao/trackdao.h"

#include <QtDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QtSql>
#include <QImage>
#include <QRegExp>
#include <QChar>

#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "library/queryutil.h"
#include "util/db/sqlstringformatter.h"
#include "util/db/sqllikewildcards.h"
#include "util/db/sqllikewildcardescaper.h"
#include "util/db/sqltransaction.h"
#include "library/coverart.h"
#include "library/coverartutils.h"
#include "library/dao/trackschema.h"
#include "library/crate/cratestorage.h"
#include "library/dao/cuedao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/analysisdao.h"
#include "library/dao/libraryhashdao.h"
#include "library/coverartcache.h"
#include "track/beatfactory.h"
#include "track/beats.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "track/globaltrackcache.h"
#include "track/tracknumbers.h"
#include "util/assert.h"
#include "util/file.h"
#include "util/logger.h"
#include "util/timer.h"
#include "util/math.h"


namespace {

mixxx::Logger kLogger("TrackDAO");

enum { UndefinedRecordIndex = -2 };

void markTrackLocationsAsDeleted(QSqlDatabase database, const QString& directory) {
    //qDebug() << "TrackDAO::markTrackLocationsAsDeleted" << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directory << "as deleted.";
    }
}

} // anonymous namespace

TrackDAO::TrackDAO(CueDAO& cueDao,
                   PlaylistDAO& playlistDao,
                   AnalysisDao& analysisDao,
                   LibraryHashDAO& libraryHashDao,
                   UserSettingsPointer pConfig)
        : m_cueDao(cueDao),
          m_playlistDao(playlistDao),
          m_analysisDao(analysisDao),
          m_libraryHashDao(libraryHashDao),
          m_pConfig(pConfig),
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
    qDebug() << "TrackDAO::finish()";

    // clear out played information on exit
    // crash prevention: if mixxx crashes, played information will be maintained
    qDebug() << "Clearing played information for this session";
    QSqlQuery query(m_database);
    if (!query.exec("UPDATE library SET played=0 where played>0")) {
        // Note: without where, this call updates every row which takes long
        LOG_FAILED_QUERY(query)
                << "Error clearing played value";
    }

    // Do housekeeping on the LibraryHashes/track_locations tables.
    qDebug() << "Cleaning LibraryHashes/track_locations tables.";
    SqlTransaction transaction(m_database);
    QStringList deletedHashDirs = m_libraryHashDao.getDeletedDirectories();

    // Delete any LibraryHashes directories that have been marked as deleted.
    m_libraryHashDao.removeDeletedDirectoryHashes();

    // And mark the corresponding tracks in track_locations in the deleted
    // directories as deleted.
    // TODO(XXX) This doesn't handle sub-directories of deleted directories.
    for (const auto& dir: deletedHashDirs) {
        markTrackLocationsAsDeleted(m_database, dir);
    }
    transaction.commit();
}

/** Retrieve the track id for the track that's located at "location" on disk.
    @return the track id for the track located at location, or -1 if the track
            is not in the database.
*/
TrackId TrackDAO::getTrackId(const QString& absoluteFilePath) {

    TrackId trackId;

    QSqlQuery query(m_database);
    query.prepare("SELECT library.id FROM library INNER JOIN track_locations ON library.location = track_locations.id WHERE track_locations.location=:location");
    query.bindValue(":location", absoluteFilePath);
    if (query.exec()) {
        if (query.next()) {
            trackId = TrackId(query.value(query.record().indexOf("id")));
        } else {
            qDebug() << "TrackDAO::getTrackId(): Track location not found in library:" << absoluteFilePath;
        }
    } else {
        LOG_FAILED_QUERY(query);
        return TrackId();
    }

    return trackId;
}

QList<TrackId> TrackDAO::getTrackIds(const QList<QFileInfo>& files) {
    QList<TrackId> trackIds;
    trackIds.reserve(files.size());

    QSqlQuery query(m_database);
    {
        QStringList pathList;
        pathList.reserve(files.size());
        for (const auto& file: files) {
            pathList << file.absoluteFilePath();
        }
        query.prepare(QString("SELECT library.id FROM library INNER JOIN "
                              "track_locations ON library.location = track_locations.id "
                              "WHERE track_locations.location in (%1)").arg(
                                      SqlStringFormatter::formatList(m_database, pathList)));
    }
    if (query.exec()) {
        const int idColumn = query.record().indexOf("id");
        while (query.next()) {
            trackIds.append(TrackId(query.value(idColumn)));
        }
        DEBUG_ASSERT(trackIds.size() <= files.size());
        if (trackIds.size() < files.size()) {
            qDebug() << "TrackDAO::getTrackIds(): Found only" << trackIds.size() << "of" << files.size() << "tracks in library";
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    return trackIds;
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
// not worth retrieving a whole Track.
QString TrackDAO::getTrackLocation(TrackId trackId) {
    qDebug() << "TrackDAO::getTrackLocation"
             << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    QString trackLocation = "";
    query.prepare("SELECT track_locations.location FROM track_locations "
                  "INNER JOIN library ON library.location = track_locations.id "
                  "WHERE library.id=:id");
    query.bindValue(":id", trackId.toVariant());
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

void TrackDAO::saveTrack(Track* pTrack) {
    DEBUG_ASSERT(pTrack);
    if (pTrack->isDirty()) {
        const TrackId trackId = pTrack->getId();
        // Only update the database if the track has already been added!
        if (trackId.isValid()) {
            qDebug() << "TrackDAO: Saving track"
                    << trackId
                    << pTrack->getFileInfo();
            if (updateTrack(pTrack)) {
                // BaseTrackCache must be informed separately, because the
                // track has already been disconnected and TrackDAO does
                // not receive any signals that are usually forwarded to
                // BaseTrackCache.
                DEBUG_ASSERT(!pTrack->isDirty());
                emit(trackClean(trackId));
            }
        }
    }
}

void TrackDAO::slotTrackDirty(Track* pTrack) {
    // Should not be possible.
    VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackDirty" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it has been
    // dirtied. It is invoked via a DirectConnection so we are sure that the
    // Track* has not been deleted when this is invoked. The flip side
    // of this is that this method runs in whatever thread the track was dirtied
    // from.
    TrackId trackId(pTrack->getId());
    if (trackId.isValid()) {
        emit(trackDirty(trackId));
    }
}

void TrackDAO::slotTrackClean(Track* pTrack) {
    // Should not be possible.
    VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackClean" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it has been cleaned
    // (typically after it has been saved to the database). It is invoked via a
    // DirectConnection so we are sure that the Track* has not been
    // deleted when this is invoked. The flip side of this is that this method
    // runs in whatever thread the track was cleaned from.
    TrackId trackId(pTrack->getId());
    if (trackId.isValid()) {
        emit(trackClean(trackId));
    }
}

void TrackDAO::databaseTrackAdded(TrackPointer pTrack) {
    emit(dbTrackAdded(pTrack));
}

void TrackDAO::databaseTracksMoved(QSet<TrackId> tracksMovedSetOld, QSet<TrackId> tracksMovedSetNew) {
    emit(tracksRemoved(tracksMovedSetNew));
    // results in a call of BaseTrackCache::updateTracksInIndex(trackIds);
    emit(tracksAdded(tracksMovedSetOld));
}

void TrackDAO::databaseTracksChanged(QSet<TrackId> tracksChanged) {
    // results in a call of BaseTrackCache::updateTracksInIndex(trackIds);
    emit(tracksAdded(tracksChanged));
}

void TrackDAO::slotTrackChanged(Track* pTrack) {
    // Should not be possible.
    VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
        return;
    }

    // qDebug() << "TrackDAO::slotTrackChanged" << pTrack << "ID"
    //          << pTrack->getId() << pTrack->getInfo();
    // This is a private slot that is connected to TIO's created by this
    // TrackDAO. It is a way for the track to notify us that it changed.  It is
    // invoked via a DirectConnection so we are sure that the Track*
    // has not been deleted when this is invoked. The flip side of this is that
    // this method runs in whatever thread the track was changed from.
    TrackId trackId(pTrack->getId());
    if (trackId.isValid()) {
        emit(trackChanged(trackId));
    }
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
    m_pTransaction = std::make_unique<SqlTransaction>(m_database);

    m_pQueryTrackLocationInsert = std::make_unique<QSqlQuery>(m_database);
    m_pQueryTrackLocationSelect = std::make_unique<QSqlQuery>(m_database);
    m_pQueryLibraryInsert = std::make_unique<QSqlQuery>(m_database);
    m_pQueryLibraryUpdate = std::make_unique<QSqlQuery>(m_database);
    m_pQueryLibrarySelect = std::make_unique<QSqlQuery>(m_database);

    m_pQueryTrackLocationInsert->prepare("INSERT INTO track_locations "
            "("
            "location,directory,filename,filesize,fs_deleted,needs_verification"
            ") VALUES ("
            ":location,:directory,:filename,:filesize,:fs_deleted,:needs_verification"
            ")");

    m_pQueryTrackLocationSelect->prepare("SELECT id FROM track_locations WHERE location=:location");

    m_pQueryLibraryInsert->prepare("INSERT INTO library "
            "("
            "artist,title,album,album_artist,year,genre,tracknumber,tracktotal,composer,"
            "grouping,filetype,location,comment,url,duration,rating,key,key_id,"
            "bitrate,samplerate,cuepoint,bpm,replaygain,replaygain_peak,wavesummaryhex,"
            "timesplayed,channels,mixxx_deleted,header_parsed,"
            "beats_version,beats_sub_version,beats,bpm_lock,"
            "keys_version,keys_sub_version,keys,"
            "coverart_source,coverart_type,coverart_location,coverart_hash,"
            "datetime_added"
            ") VALUES ("
            ":artist,:title,:album,:album_artist,:year,:genre,:tracknumber,:tracktotal,:composer,"
            ":grouping,:filetype,:location,:comment,:url,:duration,:rating,:key,:key_id,"
            ":bitrate,:samplerate,:cuepoint,:bpm,:replaygain,:replaygain_peak,:wavesummaryhex,"
            ":timesplayed,:channels,:mixxx_deleted,:header_parsed,"
            ":beats_version,:beats_sub_version,:beats,:bpm_lock,"
            ":keys_version,:keys_sub_version,:keys,"
            ":coverart_source,:coverart_type,:coverart_location,:coverart_hash,"
            ":datetime_added"
            ")");

    m_pQueryLibraryUpdate->prepare("UPDATE library SET mixxx_deleted = 0 "
            "WHERE id=:id");

    m_pQueryLibrarySelect->prepare("SELECT location, id, mixxx_deleted from library "
            "WHERE location=:location");
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
    m_pQueryTrackLocationInsert.reset();
    m_pQueryTrackLocationSelect.reset();
    m_pQueryLibraryInsert.reset();
    m_pQueryLibrarySelect.reset();
    m_pTransaction.reset();

    emit(tracksAdded(m_tracksAddedSet));
    m_tracksAddedSet.clear();
}

namespace {
    bool insertTrackLocation(QSqlQuery* pTrackLocationInsert, const Track& track) {
        DEBUG_ASSERT(nullptr != pTrackLocationInsert);
        const auto trackFile = track.getFileInfo();
        pTrackLocationInsert->bindValue(":location", trackFile.location());
        pTrackLocationInsert->bindValue(":directory", trackFile.directory());
        pTrackLocationInsert->bindValue(":filename", trackFile.fileName());
        pTrackLocationInsert->bindValue(":filesize", trackFile.fileSize());
        pTrackLocationInsert->bindValue(":fs_deleted", 0);
        pTrackLocationInsert->bindValue(":needs_verification", 0);
        if (pTrackLocationInsert->exec()) {
            return true;
        } else {
            LOG_FAILED_QUERY(*pTrackLocationInsert)
                << "Skip inserting duplicate track location" << trackFile.location();
            return false;
        }
    }

    // Bind common values for insert/update
    void bindTrackLibraryValues(QSqlQuery* pTrackLibraryQuery, const Track& track) {
        pTrackLibraryQuery->bindValue(":artist", track.getArtist());
        pTrackLibraryQuery->bindValue(":title", track.getTitle());
        pTrackLibraryQuery->bindValue(":album", track.getAlbum());
        pTrackLibraryQuery->bindValue(":album_artist", track.getAlbumArtist());
        pTrackLibraryQuery->bindValue(":year", track.getYear());
        pTrackLibraryQuery->bindValue(":genre", track.getGenre());
        pTrackLibraryQuery->bindValue(":composer", track.getComposer());
        pTrackLibraryQuery->bindValue(":grouping", track.getGrouping());
        pTrackLibraryQuery->bindValue(":tracknumber", track.getTrackNumber());
        pTrackLibraryQuery->bindValue(":tracktotal", track.getTrackTotal());
        pTrackLibraryQuery->bindValue(":filetype", track.getType());
        pTrackLibraryQuery->bindValue(":comment", track.getComment());
        pTrackLibraryQuery->bindValue(":url", track.getURL());
        pTrackLibraryQuery->bindValue(":duration", track.getDuration());
        pTrackLibraryQuery->bindValue(":rating", track.getRating());
        pTrackLibraryQuery->bindValue(":bitrate", track.getBitrate());
        pTrackLibraryQuery->bindValue(":samplerate", track.getSampleRate());
        pTrackLibraryQuery->bindValue(":cuepoint", track.getCuePoint().getPosition());
        pTrackLibraryQuery->bindValue(":bpm_lock", track.isBpmLocked()? 1 : 0);
        pTrackLibraryQuery->bindValue(":replaygain", track.getReplayGain().getRatio());
        pTrackLibraryQuery->bindValue(":replaygain_peak", track.getReplayGain().getPeak());
        pTrackLibraryQuery->bindValue(":channels", track.getChannels());

        pTrackLibraryQuery->bindValue(":header_parsed", track.isMetadataSynchronized() ? 1 : 0);

        const PlayCounter playCounter(track.getPlayCounter());
        pTrackLibraryQuery->bindValue(":timesplayed", playCounter.getTimesPlayed());
        pTrackLibraryQuery->bindValue(":played", playCounter.isPlayed() ? 1 : 0);

        const CoverInfoRelative coverInfo(track.getCoverInfo());
        pTrackLibraryQuery->bindValue(":coverart_source", coverInfo.source);
        pTrackLibraryQuery->bindValue(":coverart_type", coverInfo.type);
        pTrackLibraryQuery->bindValue(":coverart_location", coverInfo.coverLocation);
        pTrackLibraryQuery->bindValue(":coverart_hash", coverInfo.hash);

        QByteArray beatsBlob;
        QString beatsVersion;
        QString beatsSubVersion;
        // Fall back on cached BPM
        double dBpm = track.getBpm();
        const BeatsPointer pBeats(track.getBeats());
        if (!pBeats.isNull()) {
            beatsBlob = pBeats->toByteArray();
            beatsVersion = pBeats->getVersion();
            beatsSubVersion = pBeats->getSubVersion();
            dBpm = pBeats->getBpm();
        }
        pTrackLibraryQuery->bindValue(":bpm", dBpm);
        pTrackLibraryQuery->bindValue(":beats_version", beatsVersion);
        pTrackLibraryQuery->bindValue(":beats_sub_version", beatsSubVersion);
        pTrackLibraryQuery->bindValue(":beats", beatsBlob);

        QByteArray keysBlob;
        QString keysVersion;
        QString keysSubVersion;
        QString keyText;
        mixxx::track::io::key::ChromaticKey key = mixxx::track::io::key::INVALID;
        const Keys keys(track.getKeys());
        if (keys.isValid()) {
            keysBlob = keys.toByteArray();
            keysVersion = keys.getVersion();
            keysSubVersion = keys.getSubVersion();
            key = keys.getGlobalKey();
            keyText = KeyUtils::getGlobalKeyText(keys);
        }
        pTrackLibraryQuery->bindValue(":keys", keysBlob);
        pTrackLibraryQuery->bindValue(":keys_version", keysVersion);
        pTrackLibraryQuery->bindValue(":keys_sub_version", keysSubVersion);
        pTrackLibraryQuery->bindValue(":key", keyText);
        pTrackLibraryQuery->bindValue(":key_id", static_cast<int>(key));
    }

    bool insertTrackLibrary(QSqlQuery* pTrackLibraryInsert, const Track& track, DbId trackLocationId, QDateTime trackDateAdded) {
        bindTrackLibraryValues(pTrackLibraryInsert, track);

        if (!track.getDateAdded().isNull()) {
            qDebug() << "insertTrackLibrary: Track was added" << "track.getDateAdded()" << "and purged";
        }
        pTrackLibraryInsert->bindValue(":datetime_added", trackDateAdded);

        // Written only once upon insert
        pTrackLibraryInsert->bindValue(":location", trackLocationId.toVariant());

        // Column datetime_added is set implicitly
        //pTrackLibraryInsert->bindValue(":datetime_added", track.getDateAdded());

        pTrackLibraryInsert->bindValue(":mixxx_deleted", 0);

        // We no longer store the wavesummary in the library table.
        pTrackLibraryInsert->bindValue(":wavesummaryhex", QVariant(QVariant::ByteArray));

        if (pTrackLibraryInsert->exec()) {
            return true;
        } else {
            // We failed to insert the track. Maybe it is already in the library
            // but marked deleted? Skip this track.
            LOG_FAILED_QUERY(*pTrackLibraryInsert)
                    << "Failed to insert new track into library:"
                    << track.getLocation();
            return false;
        }
    }
} // anonymous namespace

TrackId TrackDAO::addTracksAddTrack(const TrackPointer& pTrack, bool unremove) {
    DEBUG_ASSERT(pTrack);
    VERIFY_OR_DEBUG_ASSERT(m_pQueryLibraryInsert || m_pQueryTrackLocationInsert ||
        m_pQueryLibrarySelect || m_pQueryTrackLocationSelect) {
        qDebug() << "TrackDAO::addTracksAddTrack: needed SqlQuerys have not "
                "been prepared. Skipping track"
                << pTrack->getFileInfo();
        return TrackId();
    }

    qDebug() << "TrackDAO: Adding track"
            << pTrack->getFileInfo();

    TrackId trackId;

    // Insert the track location into the corresponding table. This will fail
    // silently if the location is already in the table because it has a UNIQUE
    // constraint.
    if (!insertTrackLocation(m_pQueryTrackLocationInsert.get(), *pTrack)) {
        // Inserting into track_locations failed, so the file already
        // exists. Query for its trackLocationId.
        m_pQueryTrackLocationSelect->bindValue(":location", pTrack->getLocation());
        if (!m_pQueryTrackLocationSelect->exec()) {
            // We can't even select this, something is wrong.
            LOG_FAILED_QUERY(*m_pQueryTrackLocationSelect)
                        << "Can't find track location ID after failing to insert. Something is wrong.";
            return TrackId();
        }
        if (m_trackLocationIdColumn == UndefinedRecordIndex) {
            m_trackLocationIdColumn = m_pQueryTrackLocationSelect->record().indexOf("id");
        }
        DbId trackLocationId;
        while (m_pQueryTrackLocationSelect->next()) {
            // This loop body is executed at most once
            DEBUG_ASSERT(!trackLocationId.isValid());
            trackLocationId = DbId(
                    m_pQueryTrackLocationSelect->value(m_trackLocationIdColumn));
            DEBUG_ASSERT(trackLocationId.isValid());
        }

        m_pQueryLibrarySelect->bindValue(":location", trackLocationId.toVariant());
        if (!m_pQueryLibrarySelect->exec()) {
             LOG_FAILED_QUERY(*m_pQueryLibrarySelect)
                     << "Failed to query existing track: "
                     << pTrack->getFileInfo();
             return TrackId();
        }
        if (m_queryLibraryIdColumn == UndefinedRecordIndex) {
            QSqlRecord queryLibraryRecord = m_pQueryLibrarySelect->record();
            m_queryLibraryIdColumn = queryLibraryRecord.indexOf("id");
            m_queryLibraryMixxxDeletedColumn =
                    queryLibraryRecord.indexOf("mixxx_deleted");
        }
        while (m_pQueryLibrarySelect->next()) {
            // This loop body is executed at most once
            DEBUG_ASSERT(!trackId.isValid());
            trackId = TrackId(m_pQueryLibrarySelect->value(m_queryLibraryIdColumn));
            DEBUG_ASSERT(trackId.isValid());
        }
        // Track already included in library, but maybe marked as deleted
        bool mixxx_deleted = m_pQueryLibrarySelect->value(m_queryLibraryMixxxDeletedColumn).toBool();
        if (unremove && mixxx_deleted) {
            // Set mixxx_deleted back to 0
            m_pQueryLibraryUpdate->bindValue(":id", trackId.toVariant());
            if (!m_pQueryLibraryUpdate->exec()) {
                LOG_FAILED_QUERY(*m_pQueryLibraryUpdate)
                        << "Failed to unremove existing track: "
                        << pTrack->getFileInfo();
                return TrackId();
            }
        }
        // Regardless of whether we unremoved this track or not -- it's
        // already in the library and so we need to skip it instead of
        // adding it to m_tracksAddedSet.
        //
        // TODO(XXX) this is a little weird because the track has whatever
        // metadata the caller supplied and that metadata may differ from
        // what is already in the database. I'm ignoring this corner case.
        // rryan 10/2011
        // NOTE(uklotzde, 01/2016): It doesn't matter if the track metadata
        // has been modified (dirty=true) or not (dirty=false). By not adding
        // the track to m_tracksAddedSet we ensure that the track is not
        // marked as clean (see below). The library will be updated when
        // the last reference to the track is dropped.
    } else {
        // Inserting succeeded, so just get the last rowid.
        const DbId trackLocationId(m_pQueryTrackLocationInsert->lastInsertId());
        // Failure of this assert indicates that we were unable to insert the
        // track location into the table AND we could not retrieve the id of
        // that track location from the same table. "It shouldn't
        // happen"... unless I screwed up - Albert :)
        VERIFY_OR_DEBUG_ASSERT(trackLocationId.isValid()) {
            return TrackId();
        }

        // Time stamps are stored with timezone UTC in the database
        const auto trackDateAdded = QDateTime::currentDateTimeUtc();
        if (!insertTrackLibrary(m_pQueryLibraryInsert.get(), *pTrack, trackLocationId, trackDateAdded)) {
            return TrackId();
        }
        trackId = TrackId(m_pQueryLibraryInsert->lastInsertId());
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return TrackId();
        }
        pTrack->setDateAdded(trackDateAdded);

        m_analysisDao.saveTrackAnalyses(
                trackId,
                pTrack->getWaveform(),
                pTrack->getWaveformSummary());
        m_cueDao.saveTrackCues(
                trackId,
                pTrack->getCuePoints());

        DEBUG_ASSERT(!m_tracksAddedSet.contains(trackId));
        m_tracksAddedSet.insert(trackId);
    }

    return trackId;
}

TrackPointer TrackDAO::addTracksAddFile(const TrackFile& trackFile, bool unremove) {
    // Check that track is a supported extension.
    // TODO(uklotzde): The following check can be skipped if
    // the track is already in the library. A refactoring is
    // needed to detect this before calling addTracksAddTrack().
    if (!SoundSourceProxy::isFileSupported(trackFile)) {
        qWarning() << "TrackDAO::addTracksAddFile:"
                << "Unsupported file type"
                << trackFile.location();
        return TrackPointer();
    }

    GlobalTrackCacheResolver cacheResolver(trackFile);
    TrackPointer pTrack = cacheResolver.getTrack();
    if (!pTrack) {
        qWarning() << "TrackDAO::addTracksAddFile:"
                << "File not found"
                << trackFile.location();
        return TrackPointer();
    }
    const TrackId oldTrackId = pTrack->getId();
    if (oldTrackId.isValid()) {
        qDebug() << "TrackDAO::addTracksAddFile:"
                << "Track has already been added to the database"
                << oldTrackId;
        return pTrack;
    }
    // Keep the GlobalTrackCache locked until the id of the Track
    // object is known and has been updated in the cache.

    // Initially (re-)import the metadata for the newly created track
    // from the file.
    SoundSourceProxy(pTrack).updateTrackFromSource();
    if (!pTrack->isMetadataSynchronized()) {
        qWarning() << "TrackDAO::addTracksAddFile:"
                << "Failed to parse track metadata from file"
                << pTrack->getLocation();
        // Continue with adding the track to the library, no matter
        // if parsing the metadata from file succeeded or failed.
    }

    const TrackId newTrackId = addTracksAddTrack(pTrack, unremove);
    if (!newTrackId.isValid()) {
        qWarning() << "TrackDAO::addTracksAddTrack:"
                << "Failed to add track to database"
                << pTrack->getFileInfo();
        // GlobalTrackCache will be unlocked implicitly
        return TrackPointer();
    }
    cacheResolver.initTrackIdAndUnlockCache(newTrackId);
    DEBUG_ASSERT(pTrack->getId() == newTrackId);
    // Only newly inserted tracks must be marked as clean!
    // Existing or unremoved tracks have not been added to
    // m_tracksAddedSet and will keep their dirty flag unchanged.
    if (m_tracksAddedSet.contains(newTrackId)) {
        pTrack->markClean();
    }
    return pTrack;
}

TrackPointer TrackDAO::addSingleTrack(const TrackFile& trackFile, bool unremove) {
    addTracksPrepare();
    TrackPointer pTrack = addTracksAddFile(trackFile, unremove);
    addTracksFinish(!pTrack);
    return pTrack;
}

QList<TrackId> TrackDAO::addMultipleTracks(
        const QList<QFileInfo>& fileInfoList,
        bool unremove) {
    // Prepare to add tracks to the database.
    // This also begins an SQL transaction.
    addTracksPrepare();

    // Create a temporary database of the paths of all the imported tracks.
    QSqlQuery query(m_database);
    query.prepare("CREATE TEMP TABLE playlist_import "
                  "(add_index INTEGER PRIMARY KEY, location varchar (512))");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        addTracksFinish(true);
        return QList<TrackId>();
    }

    // Add all the track paths to this database.
    query.prepare("INSERT INTO playlist_import (add_index, location) "
                  "VALUES (:add_index, :location)");
    int index = 0;
    for (const auto& fileInfo: fileInfoList) {
        query.bindValue(":add_index", index);
        query.bindValue(":location", fileInfo.absoluteFilePath());
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }
        index++;
    }

    // If imported-playlist tracks are to be unremoved, do that for all playlist
    // tracks that were already in the database.
    if (unremove) {
        query.prepare("SELECT library.id FROM playlist_import, "
                      "track_locations, library WHERE library.location = track_locations.id "
                      "AND playlist_import.location = track_locations.location");
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
        }

        int idColumn = query.record().indexOf("id");
        QStringList idStringList;
        while (query.next()) {
            TrackId trackId(query.value(idColumn));
            idStringList.append(trackId.toString());
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
    while (query.next()) {
        int addIndex = query.value(addIndexColumn).toInt();
        addTracksAddFile(TrackFile(fileInfoList.at(addIndex)), unremove);
    }

    // Now that we have imported any tracks that were not already in the
    // library, re-select ordering by playlist_import.add_index to return
    // the list of track ids in the order that they were requested to be
    // added.
    QList<TrackId> trackIds;
    query.prepare("SELECT library.id FROM playlist_import, "
                  "track_locations, library WHERE library.location = track_locations.id "
                  "AND playlist_import.location = track_locations.location "
                  "ORDER BY playlist_import.add_index");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
    int idColumn = query.record().indexOf("id");
    while (query.next()) {
        TrackId trackId(query.value(idColumn));
        trackIds.append(trackId);
    }

    // Drop the temporary playlist-import table.
    query.prepare("DROP TABLE IF EXISTS playlist_import");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Finish adding tracks to the database.
    addTracksFinish();

    // Return the list of track IDs added to the database.
    return trackIds;
}

bool TrackDAO::onHidingTracks(
        const QList<TrackId>& trackIds) {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList.append(trackId.toString());
    }
    FwdSqlQuery query(m_database, QString(
            "UPDATE library SET mixxx_deleted=1 WHERE id in (%1)").arg(
                    idList.join(",")));
    return !query.hasError() && query.execPrepared();
}

void TrackDAO::afterHidingTracks(
        const QList<TrackId>& trackIds) {
    // This signal is received by basetrackcache to remove the tracks from cache
    emit(tracksRemoved(QSet<TrackId>::fromList(trackIds)));
}

// If a track has been manually "hidden" from Mixxx's library by the user via
// Mixxx's interface, this lets you add it back. When a track is hidden,
// mixxx_deleted in the DB gets set to 1. This clears that, and makes it show
// up in the library views again.
// This function should get called if you drag-and-drop a file that's been
// "hidden" from Mixxx back into the library view.
bool TrackDAO::onUnhidingTracks(
        const QList<TrackId>& trackIds) {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList.append(trackId.toString());
    }
    FwdSqlQuery query(m_database, QString(
            "UPDATE library SET mixxx_deleted=0 "
                  "WHERE id in (%1)").arg(idList.join(",")));
    return !query.hasError() && query.execPrepared();
}

void TrackDAO::afterUnhidingTracks(
        const QList<TrackId>& trackIds) {
    emit(tracksAdded(QSet<TrackId>::fromList(trackIds)));
}

QList<TrackId> TrackDAO::getTrackIds(const QDir& dir) {
    // Capture entries that start with the directory prefix dir.
    // dir needs to end in a slash otherwise we might match other
    // directories.
    const QString dirPath = dir.absolutePath();
    QString likeClause = SqlLikeWildcardEscaper::apply(dirPath + "/", kSqlLikeMatchAll) + kSqlLikeMatchAll;

    QSqlQuery query(m_database);
    query.prepare(QString("SELECT library.id FROM library INNER JOIN track_locations "
                          "ON library.location = track_locations.id "
                          "WHERE track_locations.location LIKE %1 ESCAPE '%2'")
                  .arg(SqlStringFormatter::format(m_database, likeClause), kSqlLikeMatchAll));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << dirPath;
    }

    QList<TrackId> trackIds;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        trackIds.append(TrackId(query.value(idColumn)));
    }

    return trackIds;
}

bool TrackDAO::onPurgingTracks(
        const QList<TrackId>& trackIds) {
    if (trackIds.empty()) {
        return true; // nothing to do
    }

    QStringList idList;
    for (const auto& trackId: trackIds) {
        GlobalTrackCacheLocker().purgeTrackId(trackId);
        idList.append(trackId.toString());
    }
    QString idListJoined = idList.join(",");

    QStringList locations;
    QSet<QString> directories;
    {
        FwdSqlQuery query(m_database, QString(
                "SELECT track_locations.location, track_locations.directory FROM "
                "track_locations INNER JOIN library ON library.location = "
                "track_locations.id WHERE library.id in (%1)").arg(idListJoined));
        if (query.hasError() || !query.execPrepared()) {
            return false;
        }

        QSqlRecord queryRecord = query.record();
        const int locationColumn = queryRecord.indexOf("location");
        const int directoryColumn = queryRecord.indexOf("directory");
        while (query.next()) {
            QString filePath = query.record().value(locationColumn).toString();
            locations << filePath;
            QString directory = query.record().value(directoryColumn).toString();
            directories << directory;
        }
        if (locations.empty()) {
            return false;
        }
    }
    {
        // Remove location from track_locations table
        FwdSqlQuery query(m_database, QString(
                "DELETE FROM track_locations "
                "WHERE location in (%1)").arg(
                        SqlStringFormatter::formatList(m_database, locations)));
        if (query.hasError() || !query.execPrepared()) {
            return false;
        }
    }
    {
        // Remove Track from library table
        FwdSqlQuery query(m_database, QString(
                "DELETE FROM library "
                "WHERE id in (%1)").arg(idListJoined));
        if (query.hasError() || !query.execPrepared()) {
            return false;
        }
    }
    {
        // invalidate the hash in LibraryHash,
        // in case the file was not deleted to detect it on a rescan
        // Note: -1 is used as return value for missing entries,
        // needs_verification is a temporary column used during the scan only.
        // TODO(XXX) delegate to libraryHashDAO
        FwdSqlQuery query(m_database, QString(
                "UPDATE LibraryHashes SET "
                "hash=-2 WHERE directory_path in (%1)").arg(
                        SqlStringFormatter::formatList(m_database, directories)));
        if (query.hasError() || !query.execPrepared()) {
            return false;
        }
    }

    return true;
}

void TrackDAO::afterPurgingTracks(
        const QList<TrackId>& trackIds) {
    QSet<TrackId> tracksRemovedSet = QSet<TrackId>::fromList(trackIds);
    emit(tracksRemoved(tracksRemovedSet));
    // notify trackmodels that they should update their cache as well.
    emit(forceModelUpdate());
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

bool setTrackTotal(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    pTrack->setTrackTotal(record.value(column).toString());
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
    pTrack->setDuration(record.value(column).toDouble());
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
    pTrack->setCuePoint(CuePosition(record.value(column).toDouble(), Cue::MANUAL));
    return false;
}

bool setTrackReplayGainRatio(const QSqlRecord& record, const int column,
                        TrackPointer pTrack) {
    mixxx::ReplayGain replayGain(pTrack->getReplayGain());
    replayGain.setRatio(record.value(column).toDouble());
    pTrack->setReplayGain(replayGain);
    return false;
}

bool setTrackReplayGainPeak(const QSqlRecord& record, const int column,
                        TrackPointer pTrack) {
    mixxx::ReplayGain replayGain(pTrack->getReplayGain());
    replayGain.setPeak(record.value(column).toDouble());
    pTrack->setReplayGain(replayGain);
    return false;
}

bool setTrackTimesPlayed(const QSqlRecord& record, const int column,
                         TrackPointer pTrack) {
    PlayCounter playCounter(pTrack->getPlayCounter());
    playCounter.setTimesPlayed(record.value(column).toInt());
    pTrack->setPlayCounter(playCounter);
    return false;
}

bool setTrackPlayed(const QSqlRecord& record, const int column,
                    TrackPointer pTrack) {
    PlayCounter playCounter(pTrack->getPlayCounter());
    playCounter.setPlayed(record.value(column).toBool());
    pTrack->setPlayCounter(playCounter);
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

bool setTrackMetadataSynchronized(const QSqlRecord& record, const int column,
                          TrackPointer pTrack) {
    pTrack->setMetadataSynchronized(record.value(column).toBool());
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
            *pTrack, beatsVersion, beatsSubVersion, beatsBlob);
    if (pBeats) {
        pTrack->setBeats(pBeats);
    } else {
        pTrack->setBpm(bpm);
    }
    pTrack->setBpmLocked(bpmLocked);
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
    CoverInfoRelative coverInfo;
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

TrackPointer TrackDAO::getTrackFromDB(TrackId trackId) const {
    if (!trackId.isValid()) {
        return TrackPointer();
    }

    ScopedTimer t("TrackDAO::getTrackFromDB");
    QSqlQuery query(m_database);

    ColumnPopulator columns[] = {
        // Location must be first.
        { "track_locations.location", nullptr },
        { "artist", setTrackArtist },
        { "title", setTrackTitle },
        { "album", setTrackAlbum },
        { "album_artist", setTrackAlbumArtist },
        { "year", setTrackYear },
        { "genre", setTrackGenre },
        { "composer", setTrackComposer },
        { "grouping", setTrackGrouping },
        { "tracknumber", setTrackNumber },
        { "tracktotal", setTrackTotal },
        { "filetype", setTrackFiletype },
        { "rating", setTrackRating },
        { "comment", setTrackComment },
        { "url", setTrackUrl },
        { "duration", setTrackDuration },
        { "bitrate", setTrackBitrate },
        { "samplerate", setTrackSampleRate },
        { "cuepoint", setTrackCuePoint },
        { "replaygain", setTrackReplayGainRatio },
        { "replaygain_peak", setTrackReplayGainPeak },
        { "channels", setTrackChannels },
        { "timesplayed", setTrackTimesPlayed },
        { "played", setTrackPlayed },
        { "datetime_added", setTrackDateAdded },
        { "header_parsed", setTrackMetadataSynchronized },

        // Beat detection columns are handled by setTrackBeats. Do not change
        // the ordering of these columns or put other columns in between them!
        { "bpm", setTrackBeats },
        { "beats_version", nullptr },
        { "beats_sub_version", nullptr },
        { "beats", nullptr },
        { "bpm_lock", nullptr },

        // Beat detection columns are handled by setTrackKey. Do not change the
        // ordering of these columns or put other columns in between them!
        { "key", setTrackKey },
        { "keys_version", nullptr },
        { "keys_sub_version", nullptr },
        { "keys", nullptr },

        // Cover art columns are handled by setTrackCoverInfo. Do not change the
        // ordering of these columns or put other columns in between them!
        { "coverart_source", setTrackCoverInfo },
        { "coverart_type", nullptr },
        { "coverart_location", nullptr },
        { "coverart_hash", nullptr }
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
            "WHERE library.id = %2").arg(columnsStr, trackId.toString()));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << QString("getTrack(%1)").arg(trackId.toString());
        return TrackPointer();
    }
    if (!query.next()) {
        qDebug() << "Track with id =" << trackId << "not found";
        return TrackPointer();
    }

    QSqlRecord queryRecord = query.record();
    int recordCount = queryRecord.count();
    VERIFY_OR_DEBUG_ASSERT(recordCount == columnsCount) {
        recordCount = math_min(recordCount, columnsCount);
    }

    // Location is the first column.
    const QString trackLocation(queryRecord.value(0).toString());

    GlobalTrackCacheResolver cacheResolver(TrackFile(trackLocation), trackId);
    TrackPointer pTrack = cacheResolver.getTrack();
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        // Just to be safe, but this should never happen!!
        return pTrack;
    }
    DEBUG_ASSERT(pTrack->getId() == trackId);
    if (cacheResolver.getLookupResult() == GlobalTrackCacheLookupResult::HIT) {
        // Due to race conditions the track might have been reloaded
        // from the database in the meantime. In this case we abort
        // the operation and simply return the already cached Track
        // object which is up-to-date.
        return pTrack;
    }
    DEBUG_ASSERT(cacheResolver.getLookupResult() == GlobalTrackCacheLookupResult::MISS);
    // The cache will immediately be unlocked to reduce lock contention!
    cacheResolver.unlockCache();

    // NOTE(uklotzde, 2018-02-06):
    // pTrack has only the id set and is otherwise empty. It is registered
    // in the cache with both the id and the canonical location of the file.
    // The following database query will restore and populate all remaining
    // properties while the virgin track object is already visible for other
    // threads when looking it up in the cache. This temporary inconsistency
    // is acceptable as a tradeoff for reduced lock contention. Otherwise the
    // global cache would need to be locked until the query and the population
    // of the properties has finished.

    // For every column run its populator to fill the track in with the data.
    bool shouldDirty = false;
    for (int i = 0; i < recordCount; ++i) {
        TrackPopulatorFn populator = columns[i].populator;
        if (populator != nullptr) {
            // If any populator says the track should be dirty then we dirty it.
            if ((*populator)(queryRecord, i, pTrack)) {
                shouldDirty = true;
            }
        }
    }

    // Populate track cues from the cues table.
    pTrack->setCuePoints(m_cueDao.getCuesForTrack(trackId));

    // Normally we will set the track as clean but sometimes when loading from
    // the database we need to perform upkeep that ought to be written back to
    // the database when the track is deleted.
    if (shouldDirty) {
        pTrack->markDirty();
    } else {
        pTrack->markClean();
        // Synchronize the track's metadata with the corresponding source
        // file. This import might have never been completed successfully
        // before, so just check and try for every track that has been
        // freshly loaded from the database.
        SoundSourceProxy(pTrack).updateTrackFromSource();
    }

    // Data migration: Reload track total from file tags if not initialized
    // yet. The added column "tracktotal" has been initialized with the
    // default value "//".
    // See also: Schema revision 26 in schema.xml
    if (pTrack->getTrackTotal() == "//") {
        // Reload track total from file tags if the special track
        // total migration value "//" indicates that the track total
        // is missing and needs to be reloaded.
        qDebug() << "Reloading value for 'tracktotal' once-only from file"
                " to replace the default value introduced with a previous"
                " schema upgrade";
        mixxx::TrackMetadata trackMetadata;
        if (SoundSourceProxy(pTrack).importTrackMetadata(&trackMetadata) == mixxx::MetadataSource::ImportResult::Succeeded) {
            // Copy the track total from the temporary track object
            pTrack->setTrackTotal(trackMetadata.getTrackInfo().getTrackTotal());
            // Also set the track number if it is still empty due
            // to insufficient parsing capabilities of Mixxx in
            // previous versions.
            if (!trackMetadata.getTrackInfo().getTrackNumber().isEmpty() && pTrack->getTrackNumber().isEmpty()) {
                pTrack->setTrackNumber(trackMetadata.getTrackInfo().getTrackNumber());
            }
        } else {
            qWarning() << "Failed to reload value for 'tracktotal' from file tags:"
                    << trackLocation;
        }
    }

    // Listen to dirty and changed signals
    connect(pTrack.get(), SIGNAL(dirty(Track*)),
            this, SLOT(slotTrackDirty(Track*)),
            Qt::DirectConnection);
    connect(pTrack.get(), SIGNAL(clean(Track*)),
            this, SLOT(slotTrackClean(Track*)),
            Qt::DirectConnection);
    connect(pTrack.get(), SIGNAL(changed(Track*)),
            this, SLOT(slotTrackChanged(Track*)),
            Qt::DirectConnection);

    // BaseTrackCache cares about track trackDirty/trackClean notifications
    // from TrackDAO that are triggered by the track itself. But the preceding
    // track modifications above have been sent before the TrackDAO has been
    // connected to the track's signals and need to be replayed manually.
    if (pTrack->isDirty()) {
        emit(trackDirty(trackId));
    } else {
        emit(trackClean(trackId));
    }

    return pTrack;
}

TrackPointer TrackDAO::getTrack(TrackId trackId) const {
    //qDebug() << "TrackDAO::getTrack" << QThread::currentThread() << m_database.connectionName();

    // The GlobalTrackCache is only locked while executing the following line.
    TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
    // Accessing the database is a time consuming operation that should
    // not be executed with a lock on the GlobalTrackCache. The GlobalTrackCache will
    // be locked again after the query has been executed and potential
    // race conditions will be resolved in getTrackFromDB()
    return pTrack ? pTrack : getTrackFromDB(trackId);
}

// Saves a track's info back to the database
bool TrackDAO::updateTrack(Track* pTrack) {
    const TrackId trackId = pTrack->getId();
    DEBUG_ASSERT(trackId.isValid());

    qDebug() << "TrackDAO:"
            << "Updating track in database"
            << trackId
            << pTrack->getFileInfo();

    SqlTransaction transaction(m_database);
    // PerformanceTimer time;
    // time.start();

    QSqlQuery query(m_database);

    // Update everything but "location", since that's what we identify the track by.
    query.prepare("UPDATE library SET "
            "artist=:artist,"
            "title=:title,"
            "album=:album,"
            "album_artist=:album_artist,"
            "year=:year,"
            "genre=:genre,"
            "composer=:composer,"
            "grouping=:grouping,"
            "filetype=:filetype,"
            "tracknumber=:tracknumber,"
            "tracktotal=:tracktotal,"
            "comment=:comment,"
            "url=:url,"
            "duration=:duration,"
            "rating=:rating,"
            "key=:key,"
            "key_id=:key_id,"
            "bitrate=:bitrate,"
            "samplerate=:samplerate,"
            "cuepoint=:cuepoint,"
            "bpm=:bpm,"
            "replaygain=:replaygain,"
            "replaygain_peak=:replaygain_peak,"
            "timesplayed=:timesplayed,"
            "played=:played,"
            "channels=:channels,"
            "header_parsed=:header_parsed,"
            "beats_version=:beats_version,"
            "beats_sub_version=:beats_sub_version,"
            "beats=:beats,"
            "bpm_lock=:bpm_lock,"
            "keys_version=:keys_version,"
            "keys_sub_version=:keys_sub_version,"
            "keys=:keys,"
            "coverart_source=:coverart_source,"
            "coverart_type=:coverart_type,"
            "coverart_location=:coverart_location,"
            "coverart_hash=:coverart_hash"
            " WHERE id=:track_id");

    query.bindValue(":track_id", trackId.toVariant());
    bindTrackLibraryValues(&query, *pTrack);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qWarning() << "updateTrack had no effect: trackId" << trackId << "invalid";
        return false;
    }

    //qDebug() << "Update track took : " << time.elapsed().formatMillisWithUnit() << "Now updating cues";
    //time.start();
    m_analysisDao.saveTrackAnalyses(
            trackId,
            pTrack->getWaveform(),
            pTrack->getWaveformSummary());
    m_cueDao.saveTrackCues(
            trackId, pTrack->getCuePoints());
    transaction.commit();

    //qDebug() << "Update track in database took: " << time.elapsed().formatMillisWithUnit();
    //time.start();
    pTrack->markClean();
    //qDebug() << "Dirtying track took: " << time.elapsed().formatMillisWithUnit();
    return true;
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

    QSqlQuery query(m_database);
    query.prepare(QString("UPDATE track_locations "
                          "SET needs_verification=0, fs_deleted=0 "
                          "WHERE location IN (%1)").arg(
                                  SqlStringFormatter::formatList(m_database, locations)));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark track locations as verified.";
    }
}

void TrackDAO::markTracksInDirectoriesAsVerified(const QStringList& directories) {
    //qDebug() << "TrackDAO::markTracksInDirectoryAsVerified" << QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare(
        QString("UPDATE track_locations "
                "SET needs_verification=0 "
                "WHERE directory IN (%1)").arg(
                        SqlStringFormatter::formatList(m_database, directories)));
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
    QSet<TrackId> trackIds;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Couldn't find unverified tracks";
    }
    while (query.next()) {
        trackIds.insert(TrackId(query.value(query.record().indexOf("id"))));
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

namespace {
    // Computed the longest match from the right of both strings
    int matchStringSuffix(const QString& str1, const QString& str2) {
        int matchLength = 0;
        int minLength = math_min(str1.length(), str2.length());
        while (matchLength < minLength) {
            if (str1[str1.length() - matchLength - 1] != str2[str2.length() - matchLength - 1]) {
                // first mismatch
                break;
            }
            ++matchLength;
        }
        return matchLength;
    }
}

// Look for moved files. Look for files that have been marked as
// "deleted on disk" and see if another "file" with the same name and
// files size exists in the track_locations table. That means the file has
// moved instead of being deleted outright, and so we can salvage your
// existing metadata that you have in your DB (like cue points, etc.).
// returns falls if canceled
bool TrackDAO::detectMovedTracks(QSet<TrackId>* pTracksMovedSetOld,
        QSet<TrackId>* pTracksMovedSetNew,
        const QStringList& addedTracks,
        volatile const bool* pCancel) {
    // This function should not start a transaction on it's own!
    // When it's called from libraryscanner.cpp, there already is a transaction
    // started!

    if (addedTracks.isEmpty()) {
        // We have no moved track.
        // We can only guarantee for new tracks that the user has not
        // edited metadata, which we have to preserve
        // TODO(xxx) resolve old duplicates
        return true;
    }

    // Query possible successors
    // NOTE: Successors are identified by filename and duration (in seconds).
    // Since duration is stored as double-precision floating-point and since it
    // is sometimes truncated to nearest integer, tolerance of 1 second is used.
    QSqlQuery newTrackQuery(m_database);
    newTrackQuery.prepare(QString(
            "SELECT library.id as track_id, track_locations.id as location_id, "
            "track_locations.location "
            "FROM library INNER JOIN track_locations ON library.location=track_locations.id "
            "WHERE track_locations.location IN (%1) AND "
            "filename=:filename AND "
            "ABS(duration - :duration) < 1 AND "
            "fs_deleted=0").arg(
                    SqlStringFormatter::formatList(m_database, addedTracks)));

    // Query tracks, where we need a successor for
    QSqlQuery oldTrackQuery(m_database);
    oldTrackQuery.prepare(
            "SELECT library.id as track_id, track_locations.id as location_id, "
            "track_locations.location, filename, duration "
            "FROM library INNER JOIN track_locations ON library.location=track_locations.id "
            "WHERE fs_deleted=1");
    VERIFY_OR_DEBUG_ASSERT(oldTrackQuery.exec()) {
        LOG_FAILED_QUERY(oldTrackQuery);
        return false;
    }
    QSqlRecord oldTrackQueryRecord = oldTrackQuery.record();
    const int oldTrackIdColumn = oldTrackQueryRecord.indexOf("track_id");
    const int oldLocationIdColumn = oldTrackQueryRecord.indexOf("location_id");
    const int oldLocationColumn = oldTrackQueryRecord.indexOf("location");
    const int filenameColumn = oldTrackQueryRecord.indexOf("filename");
    const int durationColumn = oldTrackQueryRecord.indexOf("duration");

    // For each track that's been "deleted" on disk...
    while (oldTrackQuery.next()) {
        if (*pCancel) {
            return false;
        }
        QString oldTrackLocation = oldTrackQuery.value(oldLocationColumn).toString();
        QString filename = oldTrackQuery.value(filenameColumn).toString();
        // rather use duration then filesize as an indicator of changes. The filesize
        // can change by adding more ID3v2 tags
        const int duration = oldTrackQuery.value(durationColumn).toInt();

        kLogger.info()
                << "Looking for substitute of missing track location"
                << oldTrackLocation;

        newTrackQuery.bindValue(":filename", filename);
        newTrackQuery.bindValue(":duration", duration);
        VERIFY_OR_DEBUG_ASSERT(newTrackQuery.exec()) {
            LOG_FAILED_QUERY(newTrackQuery);
            continue;
        }
        const auto newTrackIdColumn = newTrackQuery.record().indexOf("track_id");
        const auto newTrackLocationIdColumn = newTrackQuery.record().indexOf("location_id");
        const auto newTrackLocationColumn = newTrackQuery.record().indexOf("location");
        int newTrackLocationSuffixMatch = 0;
        TrackId newTrackId;
        DbId newTrackLocationId;
        QString newTrackLocation;
        while (newTrackQuery.next()) {
            const auto nextTrackLocation =
                    newTrackQuery.value(newTrackLocationColumn).toString();
            VERIFY_OR_DEBUG_ASSERT(nextTrackLocation != oldTrackLocation) {
                continue;
            }
            kLogger.info()
                    << "Found potential moved track location:"
                    << nextTrackLocation;
            const auto nextSuffixMatch =
                    matchStringSuffix(nextTrackLocation, oldTrackLocation);
            DEBUG_ASSERT(nextSuffixMatch >= filename.length());
            if (newTrackLocationSuffixMatch < nextSuffixMatch) {
                newTrackLocationSuffixMatch = nextSuffixMatch;
                newTrackId = TrackId(newTrackQuery.value(newTrackIdColumn));
                newTrackLocationId = DbId(newTrackQuery.value(newTrackLocationIdColumn));
                newTrackLocation = nextTrackLocation;
            }
        }
        if (newTrackLocation.isEmpty()) {
            kLogger.info()
                    << "Found no substitute for missing track location"
                    << oldTrackLocation;
            continue;
        }
        DEBUG_ASSERT(newTrackId.isValid());
        DEBUG_ASSERT(newTrackLocationId.isValid());
        kLogger.info()
                << "Found moved track location:"
                << oldTrackLocation
                << "->"
                << newTrackLocation;

        TrackId oldTrackId(oldTrackQuery.value(oldTrackIdColumn));
        DEBUG_ASSERT(oldTrackId.isValid());
        DbId oldTrackLocationId(oldTrackQuery.value(oldLocationIdColumn));
        DEBUG_ASSERT(oldTrackLocationId.isValid());

        // The queries ensure that the following assertions are always true (fs_deleted=0/1)!
        DEBUG_ASSERT(oldTrackId != newTrackId);
        DEBUG_ASSERT(oldTrackLocationId != newTrackLocationId);

        // The library scanner will have added a new row to the Library
        // table which corresponds to the track in the new location. We need
        // to remove that so we don't end up with two rows in the library
        // table for the same track.
        {
            QSqlQuery query(m_database);
            query.prepare("DELETE FROM library WHERE id=:newid");
            query.bindValue(":newid", newTrackId.toVariant());
            VERIFY_OR_DEBUG_ASSERT(query.exec()) {
                LOG_FAILED_QUERY(query);
                // Last chance to skip this entry, i.e. nothing has been
                // deleted or updated yet!
                continue;
            }
        }
        // Update the location foreign key for the existing row in the
        // library table to point to the correct row in the track_locations
        // table.
        {
            QSqlQuery query(m_database);
            query.prepare("UPDATE library SET location=:newloc WHERE id=:oldid");
            query.bindValue(":newloc", newTrackLocationId.toVariant());
            query.bindValue(":oldid", oldTrackId.toVariant());
            VERIFY_OR_DEBUG_ASSERT(query.exec()) {
                LOG_FAILED_QUERY(query);
            }
        }
        // Remove old, orphaned row from track_locations table
        {
            QSqlQuery query(m_database);
            query.prepare("DELETE FROM track_locations WHERE id=:id");
            query.bindValue(":id", oldTrackLocationId.toVariant());
            VERIFY_OR_DEBUG_ASSERT(query.exec()) {
                LOG_FAILED_QUERY(query);
            }
        }

        // We collect all the old tracks that has to be updated in BaseTrackCache
        pTracksMovedSetOld->insert(oldTrackId);
        // We collect collect all the new tracks the where added and deleted to BaseTrackCache
        pTracksMovedSetNew->insert(newTrackId);
    }
    return true;
}

void TrackDAO::markTracksAsMixxxDeleted(const QString& dir) {
    // Capture entries that start with the directory prefix dir.
    // dir needs to end in a slash otherwise we might match other
    // directories.
    QString likeClause = SqlLikeWildcardEscaper::apply(dir + "/", kSqlLikeMatchAll) + kSqlLikeMatchAll;

    QSqlQuery query(m_database);
    query.prepare(QString("SELECT library.id FROM library INNER JOIN track_locations "
                          "ON library.location = track_locations.id "
                          "WHERE track_locations.location LIKE %1 ESCAPE '%2'")
                  .arg(SqlStringFormatter::format(m_database, likeClause), kSqlLikeMatchAll));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << dir;
    }

    QStringList trackIds;
    const int idColumn = query.record().indexOf("id");
    while (query.next()) {
        trackIds.append(TrackId(query.value(idColumn)).toString());
    }

    query.prepare(QString("UPDATE library SET mixxx_deleted=1 "
                          "WHERE id in (%1)").arg(trackIds.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }
}

bool TrackDAO::verifyRemainingTracks(
        const QStringList& libraryRootDirs,
        volatile const bool* pCancel) {
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
        return false;
    }

    query2.prepare("UPDATE track_locations "
                   "SET fs_deleted=:fs_deleted, needs_verification=0 "
                   "WHERE location=:location");

    const int locationColumn = query.record().indexOf("location");
    QString trackLocation;
    while (query.next()) {
        trackLocation = query.value(locationColumn).toString();
        int fs_deleted = 0;
        for (const auto& dir: libraryRootDirs) {
            if (trackLocation.startsWith(dir)) {
                // Track is under the library root,
                // but was not verified.
                // This happens if the track was deleted
                // a symlink duplicate or on a non normalized
                // path like on non case sensitive file systems.
                fs_deleted = 1;
                break;
            }
        }

        if (fs_deleted == 0) {
            fs_deleted = QFile::exists(trackLocation) ? 0 : 1;
        }

        query2.bindValue(":fs_deleted", fs_deleted);
        query2.bindValue(":location", trackLocation);
        if (!query2.exec()) {
            LOG_FAILED_QUERY(query2);
        }
        emit(progressVerifyTracksOutside(trackLocation));
        if (*pCancel) {
            return false;
        }
    }
    return true;
}

struct TrackWithoutCover {
    TrackId trackId;
    QString trackLocation;
    QString directoryPath;
    QString trackAlbum;
};

void TrackDAO::detectCoverArtForTracksWithoutCover(volatile const bool* pCancel,
                                              QSet<TrackId>* pTracksChanged) {
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
                  "WHERE coverart_source IS NULL or coverart_source = 0 "
                  "ORDER BY track_locations.directory");

    QVector<TrackWithoutCover> tracksWithoutCover;

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
        track.trackId = TrackId(query.value(0));
        track.trackLocation = query.value(1).toString();
        // TODO(rryan) use QFileInfo path instead? symlinks? relative?
        track.directoryPath = query.value(2).toString();
        track.trackAlbum = query.value(3).toString();

        CoverInfo::Source source = static_cast<CoverInfo::Source>(
            query.value(4).toInt());
        VERIFY_OR_DEBUG_ASSERT(source != CoverInfo::USER_SELECTED) {
            qWarning() << "PROGRAMMING ERROR! detectCoverArtForTracksWithoutCover()"
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


    QString currentDirectoryPath;
    MDir currentDirectory;
    QLinkedList<QFileInfo> possibleCovers;

    for (const auto& track: tracksWithoutCover) {
        if (*pCancel) {
            return;
        }

        //qDebug() << "Searching for cover art for" << trackLocation;
        emit(progressCoverArt(track.trackLocation));

        TrackFile trackFile(track.trackLocation);
        if (!trackFile.checkFileExists()) {
            //qDebug() << trackLocation << "does not exist";
            continue;
        }

        QImage image(CoverArtUtils::extractEmbeddedCover(trackFile));
        if (!image.isNull()) {
            updateQuery.bindValue(":coverart_type",
                                  static_cast<int>(CoverInfo::METADATA));
            updateQuery.bindValue(":coverart_source",
                                  static_cast<int>(CoverInfo::GUESSED));
            // TODO() here we may introduce a duplicate hash code
            updateQuery.bindValue(":coverart_hash",
                                  CoverArtUtils::calculateHash(image));
            updateQuery.bindValue(":coverart_location", "");
            updateQuery.bindValue(":track_id", track.trackId.toVariant());
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

        CoverInfoRelative coverInfo = CoverArtUtils::selectCoverArtForTrack(
            trackFile, track.trackAlbum, possibleCovers);

        updateQuery.bindValue(":coverart_type",
                              static_cast<int>(coverInfo.type));
        updateQuery.bindValue(":coverart_source",
                              static_cast<int>(coverInfo.source));
        updateQuery.bindValue(":coverart_hash", coverInfo.hash);
        updateQuery.bindValue(":coverart_location", coverInfo.coverLocation);
        updateQuery.bindValue(":track_id", track.trackId.toVariant());
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
    const TrackId trackId = getTrackId(trackLocation);
    const bool trackAlreadyInLibrary = trackId.isValid();
    if (pAlreadyInLibrary) {
        *pAlreadyInLibrary = trackAlreadyInLibrary;
    }

    TrackPointer pTrack;
    if (trackAlreadyInLibrary) {
        pTrack = getTrack(trackId);
        if (!pTrack) {
            qWarning() << "Failed to load track"
                    << trackLocation;
            return pTrack;
        }
    } else {
        // Add Track to library -- unremove if it was previously removed.
        pTrack = addSingleTrack(trackLocation, true);
        if (!pTrack) {
            qWarning() << "Failed to add track"
                    << trackLocation;
            return pTrack;
        }
        DEBUG_ASSERT(pTrack);
        // If the track wasn't in the library already then it has not yet been
        // checked for cover art. If processCoverArt is true then we should request
        // cover processing via CoverArtCache asynchronously.
        if (processCoverArt) {
            CoverArtCache* pCache = CoverArtCache::instance();
            if (pCache != nullptr) {
                pCache->requestGuessCover(pTrack);
            }
        }
    }

    return pTrack;
}

TrackFile TrackDAO::relocateCachedTrack(
        TrackId trackId,
        TrackFile fileInfo) {
    QString trackLocation = getTrackLocation(trackId);
    if (trackLocation.isEmpty()) {
        // not found
        return fileInfo;
    } else {
        return TrackFile(trackLocation);
    }
}
