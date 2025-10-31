#include "library/dao/trackdao.h"

#include <QChar>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QtDebug>

#ifdef __SQLITE3__
#include <sqlite3.h>
#endif // __SQLITE3__

#include "library/coverart.h"
#include "library/coverartutils.h"
#include "library/dao/analysisdao.h"
#include "library/dao/cuedao.h"
#include "library/dao/libraryhashdao.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/library_prefs.h"
#include "library/queryutil.h"
#include "moc_trackdao.cpp"
#include "sources/soundsourceproxy.h"
#include "track/beats.h"
#include "track/globaltrackcache.h"
#include "track/keyfactory.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "util/assert.h"
#include "util/datetime.h"
#include "util/db/fwdsqlquery.h"
#include "util/db/sqlite.h"
#include "util/db/sqlstringformatter.h"
#include "util/db/sqltransaction.h"
#include "util/fileaccess.h"
#include "util/fileinfo.h"
#include "util/logger.h"
#include "util/math.h"
#include "util/qt.h"
#include "util/timer.h"

namespace {

mixxx::Logger kLogger("TrackDAO");

enum { UndefinedRecordIndex = -2 };

void markTrackLocationsAsDeleted(const QSqlDatabase& database, const QString& directory) {
    // kLogger.debug()<< "markTrackLocationsAsDeleted" <<
    // QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(database);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1 "
                  "WHERE directory=:directory");
    query.bindValue(":directory", directory);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directory << "as deleted.";
        DEBUG_ASSERT(!"Failed query");
    }
}

QString joinTrackIdList(const QSet<TrackId>& trackIds) {
    QStringList trackIdList;
    trackIdList.reserve(trackIds.size());
    for (const auto& trackId : trackIds) {
        trackIdList.append(trackId.toString());
    }
    return trackIdList.join(QChar(','));
}

QString locationPathPrefixFromRootDir(const QDir& rootDir) {
    // Appending '/' is required to disambiguate files from parent
    // directories, e.g. "a/b.mp3" and "a/b/c.mp3" where "a/b" would
    // match both instead of only files in the parent directory "a/b/".
    DEBUG_ASSERT(!mixxx::FileInfo(rootDir).location().endsWith('/'));
    return mixxx::FileInfo(rootDir).location() + '/';
}

QSet<QString> collectTrackLocations(FwdSqlQuery& query) {
    QSet<QString> locations;
    const int locationColumn = query.record().indexOf(LIBRARYTABLE_LOCATION);
    while (query.next()) {
        locations.insert(query.fieldValue(locationColumn).toString());
    }
    return locations;
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
    connect(&m_playlistDao,
            &PlaylistDAO::tracksRemovedFromPlayedHistory,
            this,
            [this](const QSet<TrackId>& playedTrackIds) {
                if (playedTrackIds.isEmpty()) {
                    // Nothing to do
                    return;
                }
                VERIFY_OR_DEBUG_ASSERT(updatePlayCounterFromPlayedHistory(playedTrackIds)) {
                    return;
                }
            });
}

TrackDAO::~TrackDAO() {
    kLogger.debug() << "~TrackDAO()";
    //clear all leftover Transactions and rollback the db
    addTracksFinish(true);
}

void TrackDAO::finish() {
    kLogger.debug() << "finish()";

    // clear out played information on exit
    // crash prevention: if mixxx crashes, played information will be maintained
    kLogger.debug() << "Clearing played information for this session";
    QSqlQuery query(m_database);
    if (!query.exec("UPDATE library SET played=0 where played>0")) {
        // Note: without where, this call updates every row which takes long
        LOG_FAILED_QUERY(query)
                << "Error clearing played value";
    }

    // Do housekeeping on the LibraryHashes/track_locations tables.
    kLogger.debug() << "Cleaning LibraryHashes/track_locations tables.";
    SqlTransaction transaction(m_database);
    const QStringList deletedHashDirs = m_libraryHashDao.getDeletedDirectories();

    // Delete any LibraryHashes directories that have been marked as deleted.
    m_libraryHashDao.removeDeletedDirectoryHashes();

    // And mark the corresponding tracks in track_locations in the deleted
    // directories as deleted.
    // TODO(XXX) This doesn't handle sub-directories of deleted directories.
    for (const auto& dir : deletedHashDirs) {
        markTrackLocationsAsDeleted(m_database, dir);
    }
    transaction.commit();
}

TrackId TrackDAO::getTrackIdByLocation(const QString& location) const {
    if (location.isEmpty()) {
        return {};
    }

    QSqlQuery query(m_database);
    query.prepare(
            "SELECT library.id FROM library "
            "INNER JOIN track_locations ON library.location = track_locations.id "
            "WHERE track_locations.location=:location");
    query.bindValue(":location", location);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
        return {};
    }
    if (!query.next()) {
        kLogger.debug() << "getTrackId(): Track location not found "
                           "in library:"
                        << location;
        return {};
    }
    const auto trackId = TrackId(query.value(query.record().indexOf(LIBRARYTABLE_ID)));
    DEBUG_ASSERT(trackId.isValid());
    return trackId;
}

QList<TrackId> TrackDAO::resolveTrackIds(
        const QList<QUrl>& urls,
        ResolveTrackIdFlags flags) {
    QStringList pathList;
    pathList.reserve(urls.size());
    for (const auto& url : urls) {
        const QString urlStr = url.isLocalFile() ? url.toLocalFile() : url.toString();
        pathList << "(" + SqlStringFormatter::format(m_database, urlStr) + ")";
    }

    return resolveTrackIds(pathList, flags);
}

QList<TrackId> TrackDAO::resolveTrackIds(
        const QList<mixxx::FileInfo>& fileInfos,
        ResolveTrackIdFlags flags) {
    QStringList pathList;
    pathList.reserve(fileInfos.size());
    for (const auto& fileInfo : fileInfos) {
        pathList << "(" + SqlStringFormatter::format(m_database, fileInfo.location()) + ")";
    }

    return resolveTrackIds(pathList, flags);
}

QList<TrackId> TrackDAO::resolveTrackIds(
        const QStringList& pathList,
        ResolveTrackIdFlags flags) {
    QList<TrackId> trackIds;
    trackIds.reserve(pathList.size());
    // Create a temporary database of the paths of all the imported tracks.
    QSqlQuery query(m_database);
    query.prepare(
            "CREATE TEMP TABLE playlist_import "
            "(location varchar (512))");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
        return trackIds;
    }

    // Add all the track paths temporary to this database.
    query.prepare(
            "INSERT INTO playlist_import (location) "
            "VALUES " + pathList.join(','));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
    }

    if (flags & ResolveTrackIdFlag::AddMissing) {
        // Prepare to add tracks to the database.
        // This also begins an SQL transaction.
        addTracksPrepare();

        // Any tracks not already in the database need to be added.
        query.prepare("SELECT location FROM playlist_import "
                "WHERE NOT EXISTS (SELECT location FROM track_locations "
                "WHERE playlist_import.location = track_locations.location)");
        if (!query.exec()) {
            LOG_FAILED_QUERY(query);
            DEBUG_ASSERT(!"Failed query");
        }
        const int locationColumn = query.record().indexOf(LIBRARYTABLE_LOCATION);
        while (query.next()) {
            QString location = query.value(locationColumn).toString();
            addTracksAddFile(location, true);
        }

        // Finish adding tracks to the database.
        addTracksFinish();
    }

    query.prepare(
            "SELECT library.id FROM playlist_import "
            "INNER JOIN track_locations ON playlist_import.location = track_locations.location "
            "INNER JOIN library ON library.location = track_locations.id "
            // the order by clause enforces the native sorting which is used anyway
            // hopefully optimized away. TODO() verify.
            "ORDER BY playlist_import.ROWID");

    // Old syntax for a shorter but less readable query. TODO() check performance gain
    // query.prepare(
    //    "SELECT library.id FROM playlist_import, "
    //    "track_locations, library WHERE library.location = track_locations.id "
    //    "AND playlist_import.location = track_locations.location");
    //    "ORDER BY playlist_import.ROWID");

    if (query.exec()) {
        const int idColumn = query.record().indexOf(LIBRARYTABLE_ID);
        while (query.next()) {
            trackIds.append(TrackId(query.value(idColumn)));
        }
        DEBUG_ASSERT(trackIds.size() <= pathList.size());
        if (trackIds.size() < pathList.size()) {
            kLogger.debug() << "getTrackIds(): Found only"
                            << trackIds.size()
                            << "of"
                            << pathList.size()
                            << "tracks in library";
        }
    } else {
        LOG_FAILED_QUERY(query);
    }

    // Drop the temporary playlist-import table.
    query.prepare("DROP TABLE IF EXISTS playlist_import");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
    }

    return trackIds;
}

QSet<QString> TrackDAO::getAllTrackLocations() const {
    FwdSqlQuery query(m_database, QStringLiteral("SELECT track_locations.location "
                                                 "FROM track_locations "
                                                 "INNER JOIN library "
                                                 "ON library.location = track_locations.id"));
    VERIFY_OR_DEBUG_ASSERT(!query.hasError() && query.execPrepared()) {
        LOG_FAILED_QUERY(query);
        return {};
    }
    return collectTrackLocations(query);
}

QSet<QString> TrackDAO::getAllExistingTrackLocations() const {
    FwdSqlQuery query(m_database, QStringLiteral("SELECT track_locations.location "
                                                 "FROM library INNER JOIN track_locations "
                                                 "ON library.location = track_locations.id "
                                                 "WHERE fs_deleted=0"));
    VERIFY_OR_DEBUG_ASSERT(!query.hasError() && query.execPrepared()) {
        LOG_FAILED_QUERY(query);
        return {};
    }
    return collectTrackLocations(query);
}

QSet<QString> TrackDAO::getAllMissingTrackLocations() const {
    FwdSqlQuery query(m_database, QStringLiteral("SELECT track_locations.location "
                                                 "FROM library INNER JOIN track_locations "
                                                 "ON library.location = track_locations.id "
                                                 "WHERE fs_deleted=1"));
    VERIFY_OR_DEBUG_ASSERT(!query.hasError() && query.execPrepared()) {
        LOG_FAILED_QUERY(query);
        return {};
    }
    return collectTrackLocations(query);
}

// Some code (eg. drag and drop) needs to just get a track's location, and it's
// not worth retrieving a whole Track.
QString TrackDAO::getTrackLocation(TrackId trackId) const {
    kLogger.debug() << "getTrackLocation"
                    << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    QString trackLocation = "";
    query.prepare("SELECT track_locations.location FROM track_locations "
                  "INNER JOIN library ON library.location = track_locations.id "
                  "WHERE library.id=:id");
    query.bindValue(":id", trackId.toVariant());
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
        return "";
    }
    const int locationColumn = query.record().indexOf(LIBRARYTABLE_LOCATION);
    while (query.next()) {
        trackLocation = query.value(locationColumn).toString();
    }

    return trackLocation;
}

bool TrackDAO::saveTrack(Track* pTrack) const {
    VERIFY_OR_DEBUG_ASSERT(pTrack) {
        return false;
    }
    DEBUG_ASSERT(pTrack->isDirty());

    const TrackId trackId = pTrack->getId();
    DEBUG_ASSERT(trackId.isValid());
    kLogger.debug() << "TrackDAO: Saving track"
                    << trackId
                    << pTrack->getLocation();
    if (!updateTrack(*pTrack)) {
        return false;
    }

    // BaseTrackCache must be informed separately, because the
    // track has already been disconnected and TrackDAO does
    // not receive any signals that are usually forwarded to
    // BaseTrackCache.
    pTrack->markClean();
    emit mixxx::thisAsNonConst(this)->trackClean(trackId);

    return true;
}

void TrackDAO::slotDatabaseTracksChanged(const QSet<TrackId>& changedTrackIds) {
    if (!changedTrackIds.isEmpty()) {
        emit tracksChanged(changedTrackIds);
    }
}

void TrackDAO::slotDatabaseTracksRelocated(const QList<RelocatedTrack>& relocatedTracks) {
    QSet<TrackId> removedTrackIds;
    QSet<TrackId> changedTrackIds;
    for (const auto& relocatedTrack : std::as_const(relocatedTracks)) {
        const auto changedTrackId = relocatedTrack.updatedTrackRef().getId();
        DEBUG_ASSERT(changedTrackId.isValid());
        DEBUG_ASSERT(!removedTrackIds.contains(changedTrackId));
        changedTrackIds.insert(changedTrackId);
        const auto removedTrackId = relocatedTrack.deletedTrackId();
        if (removedTrackId.isValid()) {
            DEBUG_ASSERT(!changedTrackIds.contains(removedTrackId));
            removedTrackIds.insert(removedTrackId);
        }
    }
    DEBUG_ASSERT(removedTrackIds.size() <= changedTrackIds.size());
    DEBUG_ASSERT(!removedTrackIds.intersects(changedTrackIds));
    if (!removedTrackIds.isEmpty()) {
        emit tracksRemoved(removedTrackIds);
    }
    if (!changedTrackIds.isEmpty()) {
        emit tracksChanged(changedTrackIds);
    }
}

void TrackDAO::addTracksPrepare() {
    if (m_pQueryLibraryInsert || m_pQueryTrackLocationInsert ||
            m_pQueryLibrarySelect || m_pQueryTrackLocationSelect ||
            m_pTransaction) {
        kLogger.debug() << "addTracksPrepare: PROGRAMMING ERROR"
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

    m_pQueryLibraryInsert->prepare(
            "INSERT INTO library "
            "("
            "artist,"
            "title,"
            "album,"
            "album_artist,"
            "year,"
            "genre,"
            "tracknumber,"
            "tracktotal,"
            "composer,"
            "grouping,"
            "filetype,"
            "location,"
            "color,"
            "comment,"
            "url,"
            "rating,"
            "key,"
            "key_id,"
            "cuepoint,"
            "bpm,"
            "replaygain,"
            "replaygain_peak,"
            "wavesummaryhex,"
            "timesplayed,"
            "last_played_at,"
            "played,"
            "mixxx_deleted,"
            "header_parsed,"
            "source_synchronized_ms,"
            "channels,"
            "samplerate,"
            "bitrate,"
            "duration,"
            "beats_version,"
            "beats_sub_version,"
            "beats,"
            "bpm_lock,"
            "keys_version,"
            "keys_sub_version,"
            "keys,"
            "coverart_source,"
            "coverart_type,"
            "coverart_location,"
            "coverart_color,"
            "coverart_digest,"
            "coverart_hash,"
            "datetime_added"
            ") VALUES ("
            ":artist,"
            ":title,"
            ":album,"
            ":album_artist,"
            ":year,"
            ":genre,"
            ":tracknumber,"
            ":tracktotal,"
            ":composer,"
            ":grouping,"
            ":filetype,"
            ":location,"
            ":color,"
            ":comment,"
            ":url,"
            ":rating,"
            ":key,"
            ":key_id,"
            ":cuepoint,"
            ":bpm,"
            ":replaygain,"
            ":replaygain_peak,"
            ":wavesummaryhex,"
            ":timesplayed,"
            ":last_played_at,"
            ":played,"
            ":mixxx_deleted,"
            ":header_parsed,"
            ":source_synchronized_ms,"
            ":channels,"
            ":samplerate,"
            ":bitrate,"
            ":duration,"
            ":beats_version,"
            ":beats_sub_version,"
            ":beats,"
            ":bpm_lock,"
            ":keys_version,"
            ":keys_sub_version,"
            ":keys,"
            ":coverart_source,"
            ":coverart_type,"
            ":coverart_location,"
            ":coverart_color,"
            ":coverart_digest,"
            ":coverart_hash,"
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

    emit tracksAdded(m_tracksAddedSet);
    m_tracksAddedSet.clear();
}

namespace {

bool insertTrackLocation(
        QSqlQuery* pTrackLocationInsert,
        const mixxx::FileInfo& fileInfo) {
    DEBUG_ASSERT(pTrackLocationInsert);
    pTrackLocationInsert->bindValue(":location", fileInfo.location());
    pTrackLocationInsert->bindValue(":directory", fileInfo.locationPath());
    pTrackLocationInsert->bindValue(":filename", fileInfo.fileName());
    pTrackLocationInsert->bindValue(":filesize", fileInfo.sizeInBytes());
    pTrackLocationInsert->bindValue(":fs_deleted", 0);
    pTrackLocationInsert->bindValue(":needs_verification", 0);
    if (pTrackLocationInsert->exec()) {
        return true;
    } else {
        LOG_FAILED_QUERY(*pTrackLocationInsert)
                << "Skip inserting duplicate track location" << fileInfo.location();
        return false;
    }
}

// Bind common values for insert/update
void bindTrackLibraryValues(
        QSqlQuery* pTrackLibraryQuery,
        const mixxx::TrackRecord& track,
        const mixxx::BeatsPointer& pBeats) {
    const mixxx::TrackMetadata& trackMetadata = track.getMetadata();
    const mixxx::TrackInfo& trackInfo = trackMetadata.getTrackInfo();
    const mixxx::AlbumInfo& albumInfo = trackMetadata.getAlbumInfo();

    pTrackLibraryQuery->bindValue(":artist", trackInfo.getArtist());
    pTrackLibraryQuery->bindValue(":title", trackInfo.getTitle());
    pTrackLibraryQuery->bindValue(":album", albumInfo.getTitle());
    pTrackLibraryQuery->bindValue(":album_artist", albumInfo.getArtist());
    pTrackLibraryQuery->bindValue(":year", trackInfo.getYear());
    pTrackLibraryQuery->bindValue(":genre", trackInfo.getGenre());
    pTrackLibraryQuery->bindValue(":composer", trackInfo.getComposer());
    pTrackLibraryQuery->bindValue(":grouping", trackInfo.getGrouping());
    pTrackLibraryQuery->bindValue(":tracknumber", trackInfo.getTrackNumber());
    pTrackLibraryQuery->bindValue(":tracktotal", trackInfo.getTrackTotal());
    pTrackLibraryQuery->bindValue(":filetype", track.getFileType());
    pTrackLibraryQuery->bindValue(":color", mixxx::RgbColor::toQVariant(track.getColor()));
    pTrackLibraryQuery->bindValue(":comment", trackInfo.getComment());
    pTrackLibraryQuery->bindValue(":url", track.getUrl());
    pTrackLibraryQuery->bindValue(":rating", track.getRating());
    pTrackLibraryQuery->bindValue(":cuepoint",
            track.getMainCuePosition().toEngineSamplePosMaybeInvalid());
    pTrackLibraryQuery->bindValue(":bpm_lock", track.getBpmLocked() ? 1 : 0);
    pTrackLibraryQuery->bindValue(":replaygain", trackInfo.getReplayGain().getRatio());
    pTrackLibraryQuery->bindValue(":replaygain_peak", trackInfo.getReplayGain().getPeak());

    pTrackLibraryQuery->bindValue(":channels",
            static_cast<uint>(trackMetadata.getStreamInfo().getSignalInfo().getChannelCount()));
    pTrackLibraryQuery->bindValue(":samplerate",
            static_cast<uint>(trackMetadata.getStreamInfo().getSignalInfo().getSampleRate()));
    pTrackLibraryQuery->bindValue(":bitrate",
            static_cast<uint>(trackMetadata.getStreamInfo().getBitrate()));
    pTrackLibraryQuery->bindValue(":duration",
            trackMetadata.getStreamInfo().getDuration().toDoubleSeconds());

    pTrackLibraryQuery->bindValue(":header_parsed",
            TrackDAO::getTrackHeaderParsedInternal(track) ? 1 : 0);
    const QDateTime sourceSynchronizedAt =
            track.getSourceSynchronizedAt();
    if (sourceSynchronizedAt.isValid()) {
        DEBUG_ASSERT(sourceSynchronizedAt.timeSpec() == Qt::UTC);
        pTrackLibraryQuery->bindValue(":source_synchronized_ms",
                sourceSynchronizedAt.toMSecsSinceEpoch());
    } else {
        pTrackLibraryQuery->bindValue(":source_synchronized_ms",
                QVariant());
    }

    const PlayCounter& playCounter = track.getPlayCounter();
    pTrackLibraryQuery->bindValue(":timesplayed", playCounter.getTimesPlayed());
    pTrackLibraryQuery->bindValue(":last_played_at",
            mixxx::sqlite::writeGeneratedTimestamp(playCounter.getLastPlayedAt()));
    pTrackLibraryQuery->bindValue(":played", playCounter.isPlayed() ? 1 : 0);

    const CoverInfoRelative& coverInfo = track.getCoverInfo();
    pTrackLibraryQuery->bindValue(":coverart_source", coverInfo.source);
    pTrackLibraryQuery->bindValue(":coverart_type", coverInfo.type);
    pTrackLibraryQuery->bindValue(":coverart_location", coverInfo.coverLocation);
    pTrackLibraryQuery->bindValue(":coverart_color", mixxx::RgbColor::toQVariant(coverInfo.color));
    pTrackLibraryQuery->bindValue(":coverart_digest", coverInfo.imageDigest());
    pTrackLibraryQuery->bindValue(":coverart_hash", coverInfo.legacyHash());

    QByteArray beatsBlob;
    QString beatsVersion;
    QString beatsSubVersion;
    // Fall back on cached BPM
    mixxx::Bpm bpm = trackInfo.getBpm();
    if (pBeats) {
        beatsBlob = pBeats->toByteArray();
        beatsVersion = pBeats->getVersion();
        beatsSubVersion = pBeats->getSubVersion();
        const auto trackEndPosition = mixxx::audio::FramePos{
                trackMetadata.getStreamInfo().getDuration().toDoubleSeconds() *
                pBeats->getSampleRate()};
        bpm = pBeats->getBpmInRange(mixxx::audio::kStartFramePos, trackEndPosition);
    }
    const double bpmValue = bpm.isValid() ? bpm.value() : mixxx::Bpm::kValueUndefined;
    pTrackLibraryQuery->bindValue(":bpm", bpmValue);
    pTrackLibraryQuery->bindValue(":beats_version", beatsVersion);
    pTrackLibraryQuery->bindValue(":beats_sub_version", beatsSubVersion);
    pTrackLibraryQuery->bindValue(":beats", beatsBlob);

    const Keys keys = track.getKeys();
    QByteArray keysBlob = keys.toByteArray();
    QString keysVersion = keys.getVersion();
    QString keysSubVersion = keys.getSubVersion();
    mixxx::track::io::key::ChromaticKey key = keys.getGlobalKey();
    QString keyText = keys.getGlobalKeyText();
    pTrackLibraryQuery->bindValue(":keys", keysBlob);
    pTrackLibraryQuery->bindValue(":keys_version", keysVersion);
    pTrackLibraryQuery->bindValue(":keys_sub_version", keysSubVersion);
    pTrackLibraryQuery->bindValue(":key_id", static_cast<int>(key));
    pTrackLibraryQuery->bindValue(":key", keyText);
}

bool insertTrackLibrary(
        QSqlQuery* pTrackLibraryInsert,
        const mixxx::TrackRecord& trackRecord,
        const mixxx::BeatsPointer& pBeats,
        DbId trackLocationId,
        const mixxx::FileInfo& fileInfo,
        const QDateTime& trackDateAdded) {
    bindTrackLibraryValues(pTrackLibraryInsert, trackRecord, pBeats);

    if (!trackRecord.getDateAdded().isNull()) {
        kLogger.debug() << "insertTrackLibrary: Track"
                        << fileInfo
                        << "was added"
                        << trackRecord.getDateAdded();
    }
    pTrackLibraryInsert->bindValue(":datetime_added", trackDateAdded);

    // Written only once upon insert
    pTrackLibraryInsert->bindValue(":location", trackLocationId.toVariant());

    // Column datetime_added is set implicitly
    //pTrackLibraryInsert->bindValue(":datetime_added", track.getDateAdded());

    pTrackLibraryInsert->bindValue(":mixxx_deleted", 0);

    // We no longer store the wavesummary in the library table.
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    pTrackLibraryInsert->bindValue(":wavesummaryhex", QVariant(QMetaType(QMetaType::QByteArray)));
#else
    pTrackLibraryInsert->bindValue(":wavesummaryhex", QVariant(QVariant::ByteArray));
#endif

    if (!pTrackLibraryInsert->exec()) {
        // We failed to insert the track. Maybe it is already in the library
        // but marked deleted? Skip this track.
        LOG_FAILED_QUERY(*pTrackLibraryInsert)
                << "Failed to insert new track into library:"
                << fileInfo;
        DEBUG_ASSERT(!"Failed query");
        return false;
    }
    return true;
}

} // anonymous namespace

TrackId TrackDAO::addTracksAddTrack(const TrackPointer& pTrack, bool unremove) {
    DEBUG_ASSERT(pTrack);
    const mixxx::FileInfo fileInfo = pTrack->getFileInfo();

    if (!(m_pQueryLibraryInsert || m_pQueryTrackLocationInsert ||
                m_pQueryLibrarySelect || m_pQueryTrackLocationSelect)) {
        kLogger.debug() << "addTracksAddTrack: needed SqlQuerys have not "
                           "been prepared. Skipping track"
                        << fileInfo.location();
        DEBUG_ASSERT("Failed query");
        return TrackId();
    }

    kLogger.debug() << "TrackDAO: Adding track"
                    << fileInfo.location();

    TrackId trackId;

    // Insert the track location into the corresponding table. This will fail
    // silently if the location is already in the table because it has a UNIQUE
    // constraint.
    if (!insertTrackLocation(m_pQueryTrackLocationInsert.get(), fileInfo)) {
        DEBUG_ASSERT(pTrack->getDateAdded().isValid());
        // Inserting into track_locations failed, so the file already
        // exists. Query for its trackLocationId.
        m_pQueryTrackLocationSelect->bindValue(":location", fileInfo.location());
        if (!m_pQueryTrackLocationSelect->exec()) {
            // We can't even select this, something is wrong.
            LOG_FAILED_QUERY(*m_pQueryTrackLocationSelect)
                        << "Can't find track location ID after failing to insert. Something is wrong.";
            return TrackId();
        }
        if (m_trackLocationIdColumn == UndefinedRecordIndex) {
            m_trackLocationIdColumn =
                    m_pQueryTrackLocationSelect->record().indexOf(
                            LIBRARYTABLE_MIXXXDELETED);
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
                    << fileInfo.location();
            return TrackId();
        }
        if (m_queryLibraryIdColumn == UndefinedRecordIndex) {
            QSqlRecord queryLibraryRecord = m_pQueryLibrarySelect->record();
            m_queryLibraryIdColumn = queryLibraryRecord.indexOf(LIBRARYTABLE_ID);
            m_queryLibraryMixxxDeletedColumn =
                    queryLibraryRecord.indexOf(LIBRARYTABLE_MIXXXDELETED);
        }
        while (m_pQueryLibrarySelect->next()) {
            // This loop body is executed at most once
            DEBUG_ASSERT(!trackId.isValid());
            trackId = TrackId(m_pQueryLibrarySelect->value(m_queryLibraryIdColumn));
            DEBUG_ASSERT(trackId.isValid());
        }
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return TrackId();
        }
        pTrack->initId(trackId);
        // Track already included in library, but maybe marked as deleted
        bool mixxx_deleted = m_pQueryLibrarySelect->value(m_queryLibraryMixxxDeletedColumn).toBool();
        if (unremove && mixxx_deleted) {
            // Set mixxx_deleted back to 0
            m_pQueryLibraryUpdate->bindValue(":id", trackId.toVariant());
            if (!m_pQueryLibraryUpdate->exec()) {
                LOG_FAILED_QUERY(*m_pQueryLibraryUpdate)
                        << "Failed to unremove existing track: "
                        << fileInfo.location();
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
        const auto trackRecord = pTrack->getRecord();
        if (!insertTrackLibrary(
                    m_pQueryLibraryInsert.get(),
                    trackRecord,
                    pTrack->getBeats(),
                    trackLocationId,
                    fileInfo,
                    trackDateAdded)) {
            return TrackId();
        }
        trackId = TrackId(m_pQueryLibraryInsert->lastInsertId());
        VERIFY_OR_DEBUG_ASSERT(trackId.isValid()) {
            return TrackId();
        }
        pTrack->initId(trackId);
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

TrackPointer TrackDAO::addTracksAddFile(
        const QString& filePath,
        bool unremove) {
    const auto fileAccess = mixxx::FileAccess(mixxx::FileInfo(filePath));
    // Check that track is a supported extension.
    // TODO(uklotzde): The following check can be skipped if
    // the track is already in the library. A refactoring is
    // needed to detect this before calling addTracksAddTrack().
    if (!SoundSourceProxy::isFileSupported(fileAccess.info())) {
        kLogger.warning() << "addTracksAddFile:"
                          << "Unsupported file type"
                          << fileAccess.info().location();
        return nullptr;
    }

    auto cacheResolver = GlobalTrackCacheResolver(fileAccess);
    TrackPointer pTrack = cacheResolver.getTrack();
    switch (cacheResolver.getLookupResult()) {
    case GlobalTrackCacheLookupResult::Hit: {
        VERIFY_OR_DEBUG_ASSERT(pTrack) {
            kLogger.warning() << "addTracksAddFile:"
                              << "pTrack is null"
                              << fileAccess.info().location();
            return nullptr;
        }
        const TrackId oldTrackId = pTrack->getId();
        if (oldTrackId.isValid()) {
            kLogger.debug() << "addTracksAddFile:"
                            << "Track has already been added to the database"
                            << oldTrackId;
            DEBUG_ASSERT(pTrack->getDateAdded().isValid());
            return pTrack;
        }
        // The track is cached, but not yet in the database
        break;
    }
    case GlobalTrackCacheLookupResult::Miss:
        // An (almost) empty track object
        DEBUG_ASSERT(pTrack && !pTrack->getId().isValid());
        // Continue and populate the (almost) empty track object
        break;
    case GlobalTrackCacheLookupResult::ConflictCanonicalLocation:
        // Reject requests that would otherwise cause a caching conflict
        // by accessing the same, physical file from multiple tracks concurrently.
        DEBUG_ASSERT(!pTrack);
        DEBUG_ASSERT(cacheResolver.getTrackRef().hasCanonicalLocation());
        DEBUG_ASSERT(cacheResolver.getTrackRef().getCanonicalLocation() ==
                fileAccess.info().canonicalLocation());
        kLogger.warning()
                << "Failed to add track"
                << fileAccess.info().location()
                << "because track"
                << cacheResolver.getTrackRef().getLocation()
                << "with id"
                << cacheResolver.getTrackRef().getId()
                << "referencing the same file at"
                << cacheResolver.getTrackRef().getCanonicalLocation()
                << "is already loaded.";
        return {};
    default:
        DEBUG_ASSERT(!"unreachable");
        return {};
    }
    // Keep the GlobalTrackCache locked until the id of the Track
    // object is known and has been updated in the cache.

    // Initially (re-)import the metadata for the newly created track
    // from the file.
    SoundSourceProxy(pTrack).updateTrackFromSource(
            SoundSourceProxy::UpdateTrackFromSourceMode::Once,
            SyncTrackMetadataParams::readFromUserSettings(*m_pConfig));
    if (!pTrack->checkSourceSynchronized()) {
        kLogger.warning() << "addTracksAddFile:"
                          << "Failed to parse track metadata from file"
                          << pTrack->getLocation();
        // Continue with adding the track to the library, no matter
        // if parsing the metadata from file succeeded or failed.
    }

    const TrackId newTrackId = addTracksAddTrack(pTrack, unremove);
    if (!newTrackId.isValid()) {
        kLogger.warning() << "addTracksAddTrack:"
                          << "Failed to add track to database"
                          << pTrack->getLocation();
        // GlobalTrackCache will be unlocked implicitly
        return {};
    }
    // The track object has already been initialized with the
    // database id, but the cache is not aware of this yet.
    // Re-initializing the track object with the same id again
    // from within the cache scope is allowed.
    DEBUG_ASSERT(pTrack->getId() == newTrackId);
    cacheResolver.initTrackIdAndUnlockCache(newTrackId);
    // Only newly inserted tracks must be marked as clean!
    // Existing or unremoved tracks have not been added to
    // m_tracksAddedSet and will keep their dirty flag unchanged.
    if (m_tracksAddedSet.contains(newTrackId)) {
        pTrack->markClean();
    }
    return pTrack;
}

bool TrackDAO::hideTracks(
        const QList<TrackId>& trackIds) const {
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
    // TODO: QSet<T>::fromList(const QList<T>&) is deprecated and should be
    // replaced with QSet<T>(list.begin(), list.end()).
    // However, the proposed alternative has just been introduced in Qt
    // 5.14. Until the minimum required Qt version of Mixxx is increased,
    // we need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    emit tracksRemoved(QSet<TrackId>(trackIds.begin(), trackIds.end()));
#else
    emit tracksRemoved(QSet<TrackId>::fromList(trackIds));
#endif
}

// If a track has been manually "hidden" from Mixxx's library by the user via
// Mixxx's interface, this lets you add it back. When a track is hidden,
// mixxx_deleted in the DB gets set to 1. This clears that, and makes it show
// up in the library views again.
// This function should get called if you drag-and-drop a file that's been
// "hidden" from Mixxx back into the library view.
bool TrackDAO::unhideTracks(
        const QList<TrackId>& trackIds) const {
    QStringList idList;
    for (const auto& trackId: trackIds) {
        idList.append(trackId.toString());
    }
    FwdSqlQuery query(m_database,
            "UPDATE library SET mixxx_deleted=0 "
            "WHERE mixxx_deleted!=0 "
            "AND id in (" + idList.join(",") + ")");
    return !query.hasError() && query.execPrepared();
}

void TrackDAO::afterUnhidingTracks(
        const QList<TrackId>& trackIds) {
    // TODO: QSet<T>::fromList(const QList<T>&) is deprecated and should be
    // replaced with QSet<T>(list.begin(), list.end()).
    // However, the proposed alternative has just been introduced in Qt
    // 5.14. Until the minimum required Qt version of Mixxx is increased,
    // we need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    emit tracksAdded(QSet<TrackId>(trackIds.begin(), trackIds.end()));
#else
    emit tracksAdded(QSet<TrackId>::fromList(trackIds));
#endif
}

QList<TrackRef> TrackDAO::getAllTrackRefs(const QDir& rootDir) const {
    const QString locationPathPrefix = locationPathPrefixFromRootDir(rootDir);
    QSqlQuery query(m_database);
    query.prepare(
            QStringLiteral("SELECT library.id,track_locations.location "
                           "FROM library INNER JOIN track_locations "
                           "ON library.location=track_locations.id "
                           "WHERE "
                           "INSTR(track_locations.location,:locationPathPrefix)=1"));
    query.bindValue(":locationPathPrefix", locationPathPrefix);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << locationPathPrefix;
        DEBUG_ASSERT(!"Failed query");
    }

    QList<TrackRef> trackRefs;
    const int idColumn = query.record().indexOf(LIBRARYTABLE_ID);
    const int locationColumn = query.record().indexOf(LIBRARYTABLE_LOCATION);
    while (query.next()) {
        const auto trackId = TrackId(query.value(idColumn));
        const auto fileLocation = query.value(locationColumn).toString();
        trackRefs.append(TrackRef::fromFilePath(fileLocation, trackId));
    }

    return trackRefs;
}

bool TrackDAO::onPurgingTracks(
        const QList<TrackId>& trackIds) const {
    if (trackIds.empty()) {
        return true; // nothing to do
    }

    QStringList idList;
    idList.reserve(trackIds.size());
    for (const auto& trackId : trackIds) {
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
        const int locationColumn = queryRecord.indexOf(LIBRARYTABLE_LOCATION);
        const int directoryColumn = queryRecord.indexOf(TRACKLOCATIONSTABLE_DIRECTORY);
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
    // TODO: QSet<T>::fromList(const QList<T>&) is deprecated and should be
    // replaced with QSet<T>(list.begin(), list.end()).
    // However, the proposed alternative has just been introduced in Qt
    // 5.14. Until the minimum required Qt version of Mixxx is increased,
    // we need a version check here
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QSet<TrackId> tracksRemovedSet = QSet<TrackId>(trackIds.begin(), trackIds.end());
#else
    QSet<TrackId> tracksRemovedSet = QSet<TrackId>::fromList(trackIds);
#endif
    emit tracksRemoved(tracksRemovedSet);
    // notify trackmodels that they should update their cache as well.
    emit forceModelUpdate();
}

namespace {

typedef void (*TrackPopulatorFn)(
        const QSqlRecord& record,
        const int column,
        Track* pTrack);

void setTrackArtist(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setArtist(record.value(column).toString());
}

void setTrackTitle(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setTitle(record.value(column).toString());
}

void setTrackAlbum(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setAlbum(record.value(column).toString());
}

void setTrackAlbumArtist(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setAlbumArtist(record.value(column).toString());
}

void setTrackYear(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setYear(record.value(column).toString());
}

void setTrackGenre(const QSqlRecord& record, const int column, Track* pTrack) {
    TrackDAO::setTrackGenreInternal(pTrack, record.value(column).toString());
}

void setTrackComposer(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setComposer(record.value(column).toString());
}

void setTrackGrouping(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setGrouping(record.value(column).toString());
}

void setTrackNumber(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setTrackNumber(record.value(column).toString());
}

void setTrackTotal(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setTrackTotal(record.value(column).toString());
}

void setTrackColor(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setColor(mixxx::RgbColor::fromQVariant(record.value(column)));
}

void setTrackComment(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setComment(record.value(column).toString());
}

void setTrackUrl(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setURL(record.value(column).toString());
}

void setTrackRating(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setRating(record.value(column).toInt());
}

void setTrackCuePoint(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setMainCuePosition(mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
            record.value(column).toDouble()));
}

void setTrackReplayGainRatio(const QSqlRecord& record, const int column, Track* pTrack) {
    mixxx::ReplayGain replayGain(pTrack->getReplayGain());
    replayGain.setRatio(record.value(column).toDouble());
    pTrack->setReplayGain(replayGain);
}

void setTrackReplayGainPeak(const QSqlRecord& record, const int column, Track* pTrack) {
    mixxx::ReplayGain replayGain(pTrack->getReplayGain());
    replayGain.setPeak(record.value(column).toFloat());
    pTrack->setReplayGain(replayGain);
}

void setTrackTimesPlayed(const QSqlRecord& record, const int column, Track* pTrack) {
    PlayCounter playCounter(pTrack->getPlayCounter());
    playCounter.setTimesPlayed(record.value(column).toInt());
    pTrack->setPlayCounter(playCounter);
}

void setTrackPlayed(const QSqlRecord& record, const int column, Track* pTrack) {
    PlayCounter playCounter(pTrack->getPlayCounter());
    playCounter.setPlayedFlag(record.value(column).toBool());
    pTrack->setPlayCounter(playCounter);
}

void setTrackLastPlayedAt(const QSqlRecord& record, const int column, Track* pTrack) {
    auto playCounter = pTrack->getPlayCounter();
    playCounter.setLastPlayedAt(
            mixxx::sqlite::readGeneratedTimestamp(record.value(column)));
    pTrack->setPlayCounter(playCounter);
}

void setTrackDateAdded(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setDateAdded(
            mixxx::convertVariantToDateTime(record.value(column)));
}

void setTrackFiletype(const QSqlRecord& record, const int column, Track* pTrack) {
    pTrack->setType(record.value(column).toString());
}

void setTrackHeaderParsed(const QSqlRecord& record, const int column, Track* pTrack) {
    TrackDAO::setTrackHeaderParsedInternal(pTrack, record.value(column).toBool());
}

void setTrackSourceSynchronizedAt(const QSqlRecord& record, const int column, Track* pTrack) {
    QDateTime sourceSynchronizedAt;
    const QVariant value = record.value(column);
    // Observation (Qt 5.15): QVariant::isValid() may return true even
    // if the column value is NULL and then converts to 0 (zero). This
    // is NOT desired and therefore we MUST USE QVariant::isNull() instead!
    if (!value.isNull() && value.canConvert<quint64>()) {
        DEBUG_ASSERT(value.isValid());
        const quint64 msecsSinceEpoch = qvariant_cast<quint64>(value);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        sourceSynchronizedAt.setTimeZone(QTimeZone::UTC);
#else
        sourceSynchronizedAt.setTimeSpec(Qt::UTC);
#endif
        sourceSynchronizedAt.setMSecsSinceEpoch(msecsSinceEpoch);
    }
    pTrack->setSourceSynchronizedAt(sourceSynchronizedAt);
}

void setTrackAudioProperties(
        const QSqlRecord& record,
        const int firstColumn,
        Track* pTrack) {
    const auto channels = record.value(firstColumn).toInt();
    const auto samplerate = record.value(firstColumn + 1).toInt();
    const auto bitrate = record.value(firstColumn + 2).toInt();
    const auto duration = record.value(firstColumn + 3).toDouble();
    pTrack->setAudioProperties(
            mixxx::audio::ChannelCount(channels),
            mixxx::audio::SampleRate(samplerate),
            mixxx::audio::Bitrate(bitrate),
            mixxx::Duration::fromSeconds(duration));
}

void setTrackBeats(const QSqlRecord& record, const int column, Track* pTrack) {
    const auto bpm = mixxx::Bpm(record.value(column).toDouble());
    QString beatsVersion = record.value(column + 1).toString();
    QString beatsSubVersion = record.value(column + 2).toString();
    QByteArray beatsBlob = record.value(column + 3).toByteArray();
    if (beatsVersion.isEmpty()) {
        DEBUG_ASSERT(beatsSubVersion.isEmpty());
        DEBUG_ASSERT(beatsBlob.isEmpty());
        return;
    }
    bool bpmLocked = record.value(column + 4).toBool();
    const mixxx::BeatsPointer pBeats = mixxx::Beats::fromByteArray(
            pTrack->getSampleRate(), beatsVersion, beatsSubVersion, beatsBlob);
    if (pBeats) {
        if (bpmLocked) {
            pTrack->trySetAndLockBeats(pBeats);
        } else {
            pTrack->trySetBeats(pBeats);
        }
    } else if (bpm.isValid()) {
        // Load a temporary beat grid without offset that will be replaced by the analyzer.
        const auto pBeats = mixxx::Beats::fromConstTempo(
                pTrack->getSampleRate(), mixxx::audio::kStartFramePos, bpm);
        pTrack->trySetBeats(pBeats);
    } else {
        pTrack->trySetBeats(nullptr);
    }
}

void setTrackKey(const QSqlRecord& record, const int column, Track* pTrack) {
    QString keyText = record.value(column).toString();
    QString keysVersion = record.value(column + 1).toString();
    QString keysSubVersion = record.value(column + 2).toString();
    QByteArray keysBlob = record.value(column + 3).toByteArray();
    if (!keysVersion.isEmpty()) {
        pTrack->setKeys(KeyFactory::loadKeysFromByteArray(
                keysVersion,
                keysSubVersion,
                &keysBlob));
    } else if (!keyText.isEmpty()) {
        // Typically this happens if we are upgrading from an older (<1.12.0)
        // version of Mixxx that didn't support Keys. We treat all legacy data
        // as user-generated because that way it will be treated sensitively.
        pTrack->setKeys(KeyFactory::makeBasicKeysKeepText(
                keyText,
                mixxx::track::io::key::USER));
    }
}

void setTrackCoverInfo(const QSqlRecord& record, const int column, Track* pTrack) {
    CoverInfoRelative coverInfo;
    bool ok = false;
    coverInfo.source = static_cast<CoverInfo::Source>(
            record.value(column).toInt(&ok));
    if (!ok) {
        coverInfo.source = CoverInfo::UNKNOWN;
    }
    coverInfo.type = static_cast<CoverInfo::Type>(
            record.value(column + 1).toInt(&ok));
    if (!ok) {
        coverInfo.type = CoverInfo::NONE;
    }
    coverInfo.coverLocation = record.value(column + 2).toString();
    coverInfo.color = mixxx::RgbColor::fromQVariant(record.value(column + 3));
    coverInfo.setImageDigest(
            record.value(column + 4).toByteArray(),
            record.value(column + 5).toUInt());
    pTrack->setCoverInfo(coverInfo);
}

struct ColumnPopulator {
    const char* name;
    TrackPopulatorFn populator;
};

}  // namespace

TrackPointer TrackDAO::getTrackById(TrackId trackId) const {
    if (!trackId.isValid()) {
        return nullptr;
    }

    // The GlobalTrackCache is only locked while executing the following line.
    TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackById(trackId);
    if (pTrack) {
        return pTrack;
    }

    constexpr ColumnPopulator columns[] = {
            // Location must be first and is populated manually!
            {"track_locations.location", nullptr},
            {"artist", setTrackArtist},
            {"title", setTrackTitle},
            {"album", setTrackAlbum},
            {"album_artist", setTrackAlbumArtist},
            {"year", setTrackYear},
            {"genre", setTrackGenre},
            {"composer", setTrackComposer},
            {"grouping", setTrackGrouping},
            {"tracknumber", setTrackNumber},
            {"tracktotal", setTrackTotal},
            {"filetype", setTrackFiletype},
            {"rating", setTrackRating},
            {"color", setTrackColor},
            {"comment", setTrackComment},
            {"url", setTrackUrl},
            {"cuepoint", setTrackCuePoint},
            {"replaygain", setTrackReplayGainRatio},
            {"replaygain_peak", setTrackReplayGainPeak},
            {"timesplayed", setTrackTimesPlayed},
            {"last_played_at", setTrackLastPlayedAt},
            {"played", setTrackPlayed},
            {"datetime_added", setTrackDateAdded},
            {"header_parsed", setTrackHeaderParsed},
            {"source_synchronized_ms", setTrackSourceSynchronizedAt},

            // Audio properties are set together at once. Do not change the
            // ordering of these columns or put other columns in between them!
            {"channels", setTrackAudioProperties},
            {"samplerate", nullptr},
            {"bitrate", nullptr},
            {"duration", nullptr},

            // Beat detection columns are handled by setTrackBeats. Do not change
            // the ordering of these columns or put other columns in between them!
            {"bpm", setTrackBeats},
            {"beats_version", nullptr},
            {"beats_sub_version", nullptr},
            {"beats", nullptr},
            {"bpm_lock", nullptr},

            // Key detection columns are handled by setTrackKey. Do not change the
            // ordering of these columns or put other columns in between them!
            {"key", setTrackKey},
            {"keys_version", nullptr},
            {"keys_sub_version", nullptr},
            {"keys", nullptr},

            // Cover art columns are handled by setTrackCoverInfo. Do not change the
            // ordering of these columns or put other columns in between them!
            {"coverart_source", setTrackCoverInfo},
            {"coverart_type", nullptr},
            {"coverart_location", nullptr},
            {"coverart_color", nullptr},
            {"coverart_digest", nullptr},
            {"coverart_hash", nullptr},
    };
    constexpr int columnsCount = static_cast<int>(std::size(columns));

    // Accessing the database is a time consuming operation that should not
    // be executed with a lock on the GlobalTrackCache. The GlobalTrackCache
    // will be locked again after the query has been executed (see below)
    // and potential race conditions will be resolved.
    ScopedTimer t(QStringLiteral("TrackDAO::getTrackById"));

    QSqlRecord queryRecord;
    {
        QString columnsStr;
        int columnsSize = 0;
        for (int i = 0; i < columnsCount; ++i) {
            columnsSize += static_cast<int>(qstrlen(columns[i].name)) + 1;
        }
        columnsStr.reserve(columnsSize);
        for (int i = 0; i < columnsCount; ++i) {
            if (i > 0) {
                columnsStr.append(QChar(','));
            }
            columnsStr.append(columns[i].name);
        }

        QSqlQuery query(m_database);
        query.prepare(QString(
                "SELECT %1 FROM Library "
                "INNER JOIN track_locations ON library.location = track_locations.id "
                "WHERE library.id = %2")
                              .arg(columnsStr, trackId.toString()));
        if (!query.exec()) {
            LOG_FAILED_QUERY(query)
                    << QString("getTrack(%1)").arg(trackId.toString());
            DEBUG_ASSERT(!"Failed query");
            return nullptr;
        }

        if (!query.next()) {
            kLogger.debug() << "Track with id =" << trackId << "not found";
            return nullptr;
        }
        queryRecord = query.record();
        // Only a single record is expected
        DEBUG_ASSERT(!query.next());
    }

    { // Locking scope of cacheResolver
        // Location is the first column.
        DEBUG_ASSERT(queryRecord.count() > 0);
        const auto trackLocation = queryRecord.value(0).toString();
        const auto fileInfo = mixxx::FileInfo(trackLocation);
        const auto fileAccess = mixxx::FileAccess(fileInfo);
        // Look up the track. First by trackId again, then find duplicates using the
        // fileAccess (canonical location) and if all fails add a new track object
        // to the cache
        const auto cacheResolver = GlobalTrackCacheResolver(fileAccess, trackId);
        pTrack = cacheResolver.getTrack();
        switch (cacheResolver.getLookupResult()) {
        case GlobalTrackCacheLookupResult::Hit:
            // Due to race conditions the track might have been reloaded
            // from the database in the meantime. In this case we abort
            // the operation and simply return the already cached Track
            // object which is up-to-date.
            DEBUG_ASSERT(pTrack);
            DEBUG_ASSERT(trackId.isValid() && trackId == pTrack->getId());
            DEBUG_ASSERT(fileInfo == pTrack->getFileInfo());
            return pTrack;
        case GlobalTrackCacheLookupResult::Miss:
            // An (almost) empty track object
            DEBUG_ASSERT(pTrack);
            DEBUG_ASSERT(fileInfo == pTrack->getFileInfo());
            DEBUG_ASSERT(trackId.isValid() && trackId == pTrack->getId());
            // Continue and populate the (almost) empty track object
            break;
        case GlobalTrackCacheLookupResult::ConflictCanonicalLocation:
            // Reject requests that would otherwise cause a caching caching conflict
            // by accessing the same, physical file from multiple tracks concurrently.
            DEBUG_ASSERT(!pTrack);
            DEBUG_ASSERT(!cacheResolver.getTrackRef().hasId() ||
                    trackId != cacheResolver.getTrackRef().getId());
            DEBUG_ASSERT(cacheResolver.getTrackRef().hasCanonicalLocation());
            DEBUG_ASSERT(cacheResolver.getTrackRef().getCanonicalLocation() ==
                    fileInfo.canonicalLocation());
            kLogger.warning()
                    << "Failed to load track"
                    << trackLocation
                    << "with id"
                    << trackId
                    << "because track"
                    << cacheResolver.getTrackRef().getLocation()
                    << "with id"
                    << cacheResolver.getTrackRef().getId()
                    << "referencing the same file at"
                    << cacheResolver.getTrackRef().getCanonicalLocation()
                    << "is already loaded.";
            return nullptr;
        default:
            DEBUG_ASSERT(!"unreachable");
            return nullptr;
        }
    } // End of cacheResolver locking scope

    // NOTE(uklotzde, 2018-02-06):
    // pTrack has only the id set and maybe a canonical location of the file
    // and is registered by both as available. The rest is default constructed.
    // The following database query will restore and populate all remaining
    // properties while the virgin track object is already visible for other
    // threads when looking it up in the cache. This temporary inconsistency
    // is acceptable as a tradeoff for reduced lock contention. Otherwise the
    // global cache would need to be locked until the query and the population
    // of the properties has finished.

    // For every column run its populator to fill the track in with the data.
    {
        int recordCount = queryRecord.count();
        if (recordCount != columnsCount) {
            recordCount = math_min(recordCount, columnsCount);
            DEBUG_ASSERT(!"Failed query");
        }
        for (int i = 0; i < recordCount; ++i) {
            TrackPopulatorFn populator = columns[i].populator;
            if (populator) {
                (*populator)(queryRecord, i, pTrack.get());
            }
        }
    }

    // Populate track cues from the cues table.
    pTrack->setCuePoints(m_cueDao.getCuesForTrack(trackId));
    pTrack->markClean();

    // Synchronize the track's metadata with the corresponding source
    // file. This import might have never been completed successfully
    // before, so just check and try for every track that has been
    // freshly loaded from the database.
    auto updateTrackFromSourceMode =
            SoundSourceProxy::UpdateTrackFromSourceMode::Once;
    if (m_pConfig &&
            m_pConfig->getValue(
                    mixxx::library::prefs::kSyncTrackMetadataConfigKey,
                    false)) {
        // An implicit re-import and update is performed if the
        // user has enabled export of file tags in the preferences.
        // Either they want to keep their file tags synchronized or
        // not, no exceptions!
        updateTrackFromSourceMode =
                SoundSourceProxy::UpdateTrackFromSourceMode::Newer;
    }
    DEBUG_ASSERT(!pTrack->isDirty());
    const auto sourceSynchronizedAtBefore = pTrack->getSourceSynchronizedAt();
    const auto result =
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    updateTrackFromSourceMode,
                    SyncTrackMetadataParams::readFromUserSettings(*m_pConfig));
    if (result == SoundSourceProxy::UpdateTrackFromSourceResult::MetadataImportedAndUpdated) {
        // At least the source synchronization time stamp must have changed
        DEBUG_ASSERT(pTrack->isDirty());
        const auto sourceSynchronizedAtAfter = pTrack->getSourceSynchronizedAt();
        DEBUG_ASSERT(sourceSynchronizedAtAfter.isValid());
        if (sourceSynchronizedAtBefore.isValid()) {
            // Only log subsequent re-imports but not the initial import of metadata
            DEBUG_ASSERT(updateTrackFromSourceMode ==
                    SoundSourceProxy::UpdateTrackFromSourceMode::Newer);
            DEBUG_ASSERT(sourceSynchronizedAtBefore < sourceSynchronizedAtAfter);
            kLogger.info()
                    << "Re-imported and updated outdated track metadata in library ("
                    << sourceSynchronizedAtBefore.toString(Qt::ISODateWithMs)
                    << ") with tags from modified file ("
                    << sourceSynchronizedAtAfter.toString(Qt::ISODateWithMs)
                    << "):"
                    << pTrack->getMetadata();
        }
    }

    // Listen to signals from Track objects and forward them to
    // receivers. TrackDAO works as a relay for selected track signals
    // that allows receivers to use permanent connections with
    // TrackDAO instead of connecting to individual Track objects.
    connect(pTrack.get(),
            &Track::dirty,
            this,
            &TrackDAO::trackDirty,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(pTrack.get(),
            &Track::clean,
            this,
            &TrackDAO::trackClean,
            /*signal-to-signal*/ Qt::DirectConnection);
    connect(pTrack.get(),
            &Track::changed,
            this,
            [this](TrackId trackId) {
                // Adapt and forward signal
                emit mixxx::thisAsNonConst(this)->tracksChanged(QSet<TrackId>{trackId});
            });
    connect(pTrack.get(),
            &Track::waveformSummaryUpdated,
            this,
            [this, trackId]() {
                emit mixxx::thisAsNonConst(this)->waveformSummaryUpdated(trackId);
            });

    // BaseTrackCache cares about track trackDirty/trackClean notifications
    // from TrackDAO that are triggered by the track itself. But the preceding
    // track modifications above have been sent before the TrackDAO has been
    // connected to the track's signals and need to be replayed manually.
    if (pTrack->isDirty()) {
        emit mixxx::thisAsNonConst(this)->trackDirty(trackId);
    } else {
        emit mixxx::thisAsNonConst(this)->trackClean(trackId);
    }

    return pTrack;
}

TrackPointer TrackDAO::getTrackByRef(
        const TrackRef& trackRef) const {
    if (!trackRef.isValid()) {
        return nullptr;
    }
    const auto pTrack = GlobalTrackCacheLocker().lookupTrackByRef(trackRef);
    if (pTrack) {
        return pTrack;
    }
    auto trackId = trackRef.getId();
    if (!trackId.isValid()) {
        trackId = getTrackIdByLocation(trackRef.getLocation());
    }
    if (!trackId.isValid()) {
        kLogger.debug() << "Track not found:" << trackRef;
        return nullptr;
    }
    return getTrackById(trackId);
}

// Saves a track's info back to the database
bool TrackDAO::updateTrack(const Track& track) const {
    const TrackId trackId = track.getId();
    DEBUG_ASSERT(trackId.isValid());

    kLogger.debug() << "TrackDAO:"
                    << "Updating track in database"
                    << trackId
                    << track.getLocation();

    SqlTransaction transaction(m_database);
    // PerformanceTimer time;
    // time.start();

    QSqlQuery query(m_database);

    // Update everything but "location", since that's what we identify the track by.
    query.prepare(
            "UPDATE library SET "
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
            "color=:color,"
            "comment=:comment,"
            "url=:url,"
            "rating=:rating,"
            "key=:key,"
            "key_id=:key_id,"
            "cuepoint=:cuepoint,"
            "bpm=:bpm,"
            "replaygain=:replaygain,"
            "replaygain_peak=:replaygain_peak,"
            "timesplayed=:timesplayed,"
            "last_played_at=:last_played_at,"
            "played=:played,"
            "header_parsed=:header_parsed,"
            "source_synchronized_ms=:source_synchronized_ms,"
            "channels=:channels,"
            "bitrate=:bitrate,"
            "samplerate=:samplerate,"
            "bitrate=:bitrate,"
            "duration=:duration,"
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
            "coverart_color=:coverart_color,"
            "coverart_digest=:coverart_digest,"
            "coverart_hash=:coverart_hash "
            "WHERE id=:track_id");

    query.bindValue(":track_id", trackId.toVariant());

    const auto trackRecord = track.getRecord();
    bindTrackLibraryValues(
            &query,
            trackRecord,
            track.getBeats());

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
        return false;
    }

    if (query.numRowsAffected() == 0) {
        kLogger.warning() << "updateTrack had no effect: trackId" << trackId << "invalid";
        return false;
    }

    // kLogger.debug() << "Update track took : " <<
    // time.elapsed().formatMillisWithUnit() << "Now updating cues";
    // time.start();
    m_analysisDao.saveTrackAnalyses(
            trackId,
            track.getWaveform(),
            track.getWaveformSummary());
    m_cueDao.saveTrackCues(
            trackId, track.getCuePoints());
    transaction.commit();

    // kLogger.debug() << "Update track in database took: " <<
    // time.elapsed().formatMillisWithUnit(); time.start();
    return true;
}

// Make sure that `directory` in in track_locations table is indeed a
// directory path. This works around / removes residues of a bug where tracks
// are falsely marked missing because `directory` == `location`.
// See https://github.com/mixxxdj/mixxx/issues/14513
void TrackDAO::cleanupTrackLocationsDirectory() const {
    kLogger.debug() << "cleanupTrackLocationsDirectory"
                    << QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    // If directory == location, we remove the filename and the trailing /
    // As of now, this is the only inconsistency we know of. We might also verify
    // directory (eg. no trailing /) or recreate file name and directory from
    // location.
    query.prepare(QStringLiteral(
            "UPDATE track_locations "
            "SET directory=rtrim(rtrim(directory, filename),'/') "
            "WHERE directory == location"));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't clean up track directories.";
        DEBUG_ASSERT(!"Failed query");
    }
}

// Mark all the tracks in the library as invalid.
// That means we'll need to later check that those tracks actually
// (still) exist as part of the library scanning procedure.
void TrackDAO::invalidateTrackLocationsInLibrary() const {
    // kLogger.debug()<< "invalidateTrackLocations" <<
    // QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("UPDATE track_locations SET needs_verification = 1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in library as needing verification.";
        DEBUG_ASSERT(!"Failed query");
    }
}

void TrackDAO::markTrackLocationsAsVerified(const QStringList& locations) const {
    // kLogger.debug()<< "markTrackLocationsAsVerified" <<
    // QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare(QString("UPDATE track_locations "
                          "SET needs_verification=0, fs_deleted=0 "
                          "WHERE location IN (%1)").arg(
                                  SqlStringFormatter::formatList(m_database, locations)));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark track locations as verified.";
        DEBUG_ASSERT(!"Failed query");
    }
}

void TrackDAO::markTracksInDirectoriesAsVerified(const QStringList& directories) const {
    // kLogger.debug()<< "markTracksInDirectoryAsVerified" <<
    // QThread::currentThread() << m_database.connectionName();

    QSqlQuery query(m_database);
    query.prepare(
        QString("UPDATE track_locations "
                "SET needs_verification=0 "
                "WHERE directory IN (%1)").arg(
                        SqlStringFormatter::formatList(m_database, directories)));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark tracks in" << directories.size() << "directories as verified.";
        DEBUG_ASSERT(!"Failed query");
    }
}

void TrackDAO::markUnverifiedTracksAsDeleted() {
    // kLogger.debug()<< "markUnverifiedTracksAsDeleted" <<
    // QThread::currentThread() << m_database.connectionName();
    QSqlQuery query(m_database);
    query.prepare("SELECT library.id as id FROM library INNER JOIN track_locations ON "
                  "track_locations.id=library.location WHERE "
                  "track_locations.needs_verification=1");
    QSet<TrackId> trackIds;
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "Couldn't find unverified tracks";
        DEBUG_ASSERT(!"Failed query");
    }
    while (query.next()) {
        trackIds.insert(TrackId(query.value(query.record().indexOf(LIBRARYTABLE_MIXXXDELETED))));
    }
    emit tracksRemoved(trackIds);
    query.prepare("UPDATE track_locations "
                  "SET fs_deleted=1, needs_verification=0 "
                  "WHERE needs_verification=1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query)
                << "Couldn't mark unverified tracks as deleted.";
        DEBUG_ASSERT(!"Failed query");
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
} // namespace

// Look for moved files. Look for files that have been marked as
// 'deleted on disk' and see if another track with the same file name and
// duration exists in the track_locations table. That means the file has been
// moved instead of being deleted outright, and so we can salvage your
// existing metadata that you have in your DB (like cue points, etc.).
// Returns false if canceled.
bool TrackDAO::detectMovedTracks(
        QList<RelocatedTrack> *pRelocatedTracks,
        const QStringList& addedTracks,
        volatile const bool* pCancel) const {
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
    newTrackQuery.prepare(QStringLiteral(
            "SELECT library.id as %1, track_locations.id as %2, "
            "track_locations.location "
            "FROM library INNER JOIN track_locations ON library.location=track_locations.id "
            "WHERE track_locations.location IN (%3) AND "
            "filename=:filename AND "
            "ABS(duration - :duration) < 1 AND "
            "fs_deleted=0")
                    .arg(
                            TRACK_ID,
                            LOCATION_ID,
                            SqlStringFormatter::formatList(m_database, addedTracks)));

    // Query tracks, where we need a successor for
    QSqlQuery oldTrackQuery(m_database);
    oldTrackQuery.prepare(QStringLiteral(
            "SELECT library.id as %1, track_locations.id as %2, "
            "track_locations.location, filename, duration "
            "FROM library INNER JOIN track_locations ON library.location=track_locations.id "
            "WHERE fs_deleted=1")
                    .arg(TRACK_ID, LOCATION_ID));
    if (!oldTrackQuery.exec()) {
        LOG_FAILED_QUERY(oldTrackQuery);
        DEBUG_ASSERT(!"Failed query");
        return false;
    }
    QSqlRecord oldTrackQueryRecord = oldTrackQuery.record();
    const int oldTrackIdColumn = oldTrackQueryRecord.indexOf(TRACK_ID);
    const int oldLocationIdColumn = oldTrackQueryRecord.indexOf(LOCATION_ID);
    const int oldLocationColumn = oldTrackQueryRecord.indexOf(LIBRARYTABLE_LOCATION);
    const int filenameColumn = oldTrackQueryRecord.indexOf(TRACKLOCATIONSTABLE_FILENAME);
    const int durationColumn = oldTrackQueryRecord.indexOf(LIBRARYTABLE_DURATION);

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
        if (!newTrackQuery.exec()) {
            LOG_FAILED_QUERY(newTrackQuery);
            DEBUG_ASSERT(!"Failed query");
            continue;
        }
        const auto newTrackIdColumn = newTrackQuery.record().indexOf(TRACK_ID);
        const auto newTrackLocationIdColumn = newTrackQuery.record().indexOf(LOCATION_ID);
        const auto newTrackLocationColumn = newTrackQuery.record().indexOf(LIBRARYTABLE_LOCATION);
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
            // When we look for a successor for 'Music/Abba-1981-Greatest Hits/1-Waterloo.mp3'
            // we may have multiple candidates (same name, same duration).
            // With this suffix match ranking we'll prefer
            // 'Music/Abba/Greatest Hits/Waterloo-1.mp3' over
            // 'Music/Falko/Track1.mp3'
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

        auto missingTrackRef = TrackRef::fromFilePath(
                oldTrackLocation,
                std::move(oldTrackId));
        auto addedTrackRef = TrackRef::fromFilePath(
                newTrackLocation,
                std::move(newTrackId));
        auto relocatedTrack = RelocatedTrack(
            std::move(missingTrackRef),
            std::move(addedTrackRef));

        // The library scanner will have added a new row to the Library
        // table which corresponds to the track in the new location. We need
        // to remove that so we don't end up with two rows in the library
        // table for the same track.
        {
            QSqlQuery query(m_database);
            query.prepare("DELETE FROM library WHERE id=:newid");
            query.bindValue(":newid", relocatedTrack.deletedTrackId().toVariant());
            if (!query.exec()) {
                LOG_FAILED_QUERY(query);
                // Last chance to skip this entry, i.e. nothing has been
                // deleted or updated yet!
                DEBUG_ASSERT(!"Failed query");
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
            query.bindValue(":oldid", relocatedTrack.updatedTrackRef().getId().toVariant());
            if (!query.exec()) {
                LOG_FAILED_QUERY(query);
                DEBUG_ASSERT(!"Failed query");
            }
        }

        // Remove old, orphaned row from track_locations table
        {
            QSqlQuery query(m_database);
            query.prepare("DELETE FROM track_locations WHERE id=:id");
            query.bindValue(":id", oldTrackLocationId.toVariant());
            if (!query.exec()) {
                LOG_FAILED_QUERY(query);
                DEBUG_ASSERT(!"Failed query");
            }
        }

        if (pRelocatedTracks) {
            pRelocatedTracks->append(std::move(relocatedTrack));
        }
    }
    return true;
}

void TrackDAO::hideAllTracks(const QDir& rootDir) const {
    const QString locationPathPrefix = locationPathPrefixFromRootDir(rootDir);
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
            "SELECT library.id FROM library INNER JOIN track_locations "
            "ON library.location=track_locations.id "
            "WHERE "
            "INSTR(track_locations.location,:locationPathPrefix)=1"
            "locationPathPrefix"));
    query.bindValue(":locationPathPrefix", locationPathPrefix);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "could not get tracks within directory:" << rootDir;
        DEBUG_ASSERT(!"Failed query");
    }

    QStringList trackIds;
    const int idColumn = query.record().indexOf(LIBRARYTABLE_MIXXXDELETED);
    while (query.next()) {
        trackIds.append(TrackId(query.value(idColumn)).toString());
    }

    query.prepare(QString("UPDATE library SET mixxx_deleted=1 "
                          "WHERE id in (%1)").arg(trackIds.join(",")));
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
    }
}

bool TrackDAO::verifyRemainingTracks(
        const QList<mixxx::FileInfo>& libraryRootDirs,
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
    query.prepare(
            "SELECT location, fs_deleted "
            "FROM track_locations "
            "WHERE needs_verification = 1");
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        DEBUG_ASSERT(!"Failed query");
        return false;
    }

    query2.prepare("UPDATE track_locations "
                   "SET fs_deleted=:fs_deleted, needs_verification=0 "
                   "WHERE location=:location");

    const int fsDeletedColumn = query.record().indexOf(TRACKLOCATIONSTABLE_FSDELETED);
    const int locationColumn = query.record().indexOf(LIBRARYTABLE_LOCATION);
    QString trackLocation;
    while (query.next()) {
        trackLocation = query.value(locationColumn).toString();
        int fs_deleted = 0;
        int old_fs_deleded = query.value(fsDeletedColumn).toInt();
        for (const auto& rootDir : libraryRootDirs) {
            if (trackLocation.startsWith(rootDir.location())) {
                // Track is under the library root, but was not verified.
                // This happens if the track was deleted
                // a symlink duplicate or on a non normalized
                // path like on non-case-sensitive file systems.
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
        emit progressVerifyTracksOutside(trackLocation);
        if (fs_deleted != old_fs_deleded) {
            // Emit update so e.g. the tack model knows.
            // Otherwise the table view may still paint it with 'track missing'
            // color even though it has been re-discovered.
            TrackId id = getTrackIdByLocation(trackLocation);
            if (id.isValid()) {
                QSet<TrackId> ids{id};
                emit tracksChanged(ids);
            }
        }
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
        DEBUG_ASSERT(!"Failed query");
        return;
    }

    // We quickly iterate through the results to prevent blocking the database
    // for other operations. Issue #7713.
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
        if (source == CoverInfo::USER_SELECTED) {
            kLogger.warning() << "PROGRAMMING ERROR! detectCoverArtForTracksWithoutCover()"
                              << "got a USER_SELECTED track. Skipping.";
            DEBUG_ASSERT(!"Failed query");
            continue;
        }
        tracksWithoutCover.append(track);
    }

    QSqlQuery updateQuery(m_database);
    updateQuery.prepare(
            "UPDATE library SET "
            "coverart_type=:coverart_type,"
            "coverart_source=:coverart_source,"
            "coverart_location=:coverart_location,"
            "coverart_color=:coverart_color,"
            "coverart_digest=:coverart_digest,"
            "coverart_hash=:coverart_hash "
            "WHERE id=:track_id");

    CoverInfoGuesser coverInfoGuesser;
    for (const auto& track: tracksWithoutCover) {
        if (*pCancel) {
            return;
        }

        // kLogger.debug() << "Searching for cover art for" << trackLocation;
        emit progressCoverArt(track.trackLocation);

        const auto fileInfo = mixxx::FileInfo(track.trackLocation);
        if (!fileInfo.checkFileExists()) {
            // kLogger.debug() << fileInfo << "does not exist";
            continue;
        }

        const auto embeddedCover =
                CoverArtUtils::extractEmbeddedCover(
                        mixxx::FileAccess(fileInfo));
        const auto coverInfo =
                coverInfoGuesser.guessCoverInfo(
                        fileInfo,
                        track.trackAlbum,
                        embeddedCover);
        DEBUG_ASSERT(coverInfo.source != CoverInfo::UNKNOWN);

        updateQuery.bindValue(":track_id", track.trackId.toVariant());
        updateQuery.bindValue(":coverart_type",
                              static_cast<int>(coverInfo.type));
        updateQuery.bindValue(":coverart_source",
                              static_cast<int>(coverInfo.source));
        updateQuery.bindValue(":coverart_location", coverInfo.coverLocation);
        updateQuery.bindValue(":coverart_color", mixxx::RgbColor::toQVariant(coverInfo.color));
        updateQuery.bindValue(":coverart_digest", coverInfo.imageDigest());
        updateQuery.bindValue(":coverart_hash", coverInfo.legacyHash());

        if (!updateQuery.exec()) {
            LOG_FAILED_QUERY(updateQuery) << "failed to write file or none cover";
        } else {
            pTracksChanged->insert(track.trackId);
        }
    }
}

TrackPointer TrackDAO::getOrAddTrack(
        const TrackRef& trackRef,
        bool* pAlreadyInLibrary) {
    // This function should only be used when you do not yet know whether the
    // specified track is contained in the library. Use TrackDAO::getTrackByRef
    // instead for tracks that you already know to be in the library.
    DEBUG_ASSERT(!trackRef.hasId());
    VERIFY_OR_DEBUG_ASSERT(trackRef.hasLocation()) {
        return {};
    }
    TrackPointer pTrack = GlobalTrackCacheLocker().lookupTrackByRef(trackRef);
    if (!pTrack) {
        // track not cached
        const TrackId trackId = getTrackIdByLocation(trackRef.getLocation());
        if (trackId.isValid()) {
            // create track from library
            pTrack = getTrackById(trackId);
        }
    }
    if (pTrack) {
        if (pTrack->getId().isValid()) {
            DEBUG_ASSERT(pTrack->getDateAdded().isValid());
            if (pAlreadyInLibrary) {
                *pAlreadyInLibrary = true;
            }
            return pTrack;
        }
    }

    addTracksPrepare();
    bool unremove = true;
    const TrackPointer pAddedTrack = addTracksAddFile(trackRef.getLocation(), unremove);
    bool rollback = !pAddedTrack;
    addTracksFinish(rollback);
    if (!pAddedTrack) {
        return pAddedTrack;
    }
    if (pAlreadyInLibrary) {
        *pAlreadyInLibrary = false;
    }
    DEBUG_ASSERT(pAddedTrack->getDateAdded().isValid());
    DEBUG_ASSERT(!pTrack || pTrack == pAddedTrack);

    // If the track wasn't in the library already then it has not yet
    // been checked for cover art.
    if (!pAddedTrack->getCoverInfo().hasImage()) {
        const auto future = guessTrackCoverInfoConcurrently(pAddedTrack);
        // Don't wait for the result and keep running in the background
        Q_UNUSED(future)
    }
    return pAddedTrack;
}

mixxx::FileAccess TrackDAO::relocateCachedTrack(TrackId trackId) {
    QString trackLocation = getTrackLocation(trackId);
    if (trackLocation.isEmpty()) {
        // not found
        return mixxx::FileAccess();
    } else {
        return mixxx::FileAccess(mixxx::FileInfo(trackLocation));
    }
}

bool TrackDAO::updatePlayCounterFromPlayedHistory(
        const QSet<TrackId>& trackIds) const {
    // Invoking this function with an empty list is pointless.
    // All following database queries assume that the list is
    // not empty. Otherwise the played history of all tracks
    // might be reset!!!
    // https://github.com/mixxxdj/mixxx/issues/10617
    VERIFY_OR_DEBUG_ASSERT(!trackIds.isEmpty()) {
        return false;
    }
    // Update both timesplayed and last_played_at according to the
    // corresponding aggregated properties from the played history,
    // i.e. COUNT for the number of times a track has been played
    // and MAX for the last time it has been played.
    // NOTE: The played flag for the current session is NOT updated!
    // The current session is unaffected, because the corresponding
    // playlist cannot be deleted.
    //
    // https://www.sqlite.org/lang_update.html#upfrom
    // UPDATE-FROM is supported beginning in SQLite version 3.33.0 (2020-08-14)
    // https://github.com/mixxxdj/mixxx/issues/10482
#ifdef __SQLITE3__
    if (sqlite3_libversion_number() >= 3033000) {
#endif // __SQLITE3__
        const QString trackIdList = joinTrackIdList(trackIds);
        auto updatePlayed = FwdSqlQuery(
                m_database,
                QStringLiteral(
                        "UPDATE library SET "
                        "timesplayed=q.timesplayed,"
                        "last_played_at=q.last_played_at "
                        "FROM("
                        "SELECT "
                        "PlaylistTracks.track_id as id,"
                        "COUNT(PlaylistTracks.track_id) as timesplayed,"
                        "MAX(PlaylistTracks.pl_datetime_added) as last_played_at "
                        "FROM PlaylistTracks "
                        "JOIN Playlists ON "
                        "PlaylistTracks.playlist_id=Playlists.id "
                        "WHERE Playlists.hidden=:playlistHidden "
                        "GROUP BY PlaylistTracks.track_id"
                        ") q "
                        "WHERE library.id=q.id "
                        "AND library.id IN (%1)")
                        .arg(trackIdList));
        updatePlayed.bindValue(
                QStringLiteral(":playlistHidden"),
                QVariant(PlaylistDAO::PLHT_SET_LOG));
        VERIFY_OR_DEBUG_ASSERT(!updatePlayed.hasError()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(updatePlayed.execPrepared()) {
            return false;
        }
        auto updateNotPlayed = FwdSqlQuery(
                m_database,
                QStringLiteral(
                        "UPDATE library SET "
                        "timesplayed=0,"
                        "last_played_at=NULL "
                        "WHERE id NOT IN("
                        "SELECT PlaylistTracks.track_id "
                        "FROM PlaylistTracks "
                        "JOIN Playlists ON "
                        "PlaylistTracks.playlist_id=Playlists.id "
                        "WHERE Playlists.hidden=:playlistHidden) "
                        "AND id IN (%1)")
                        .arg(trackIdList));
        updateNotPlayed.bindValue(
                QStringLiteral(":playlistHidden"),
                QVariant(PlaylistDAO::PLHT_SET_LOG));
        VERIFY_OR_DEBUG_ASSERT(!updateNotPlayed.hasError()) {
            return false;
        }
        VERIFY_OR_DEBUG_ASSERT(updateNotPlayed.execPrepared()) {
            return false;
        }
#ifdef __SQLITE3__
    } else {
        // TODO: Remove this workaround after dropping support for Ubuntu 20.04
        auto playCounterQuery = FwdSqlQuery(
                m_database,
                QStringLiteral(
                        "SELECT "
                        "COUNT(PlaylistTracks.track_id),"
                        "MAX(PlaylistTracks.pl_datetime_added) "
                        "FROM PlaylistTracks "
                        "JOIN Playlists ON "
                        "PlaylistTracks.playlist_id=Playlists.id "
                        "WHERE Playlists.hidden=:playlistHidden "
                        "AND PlaylistTracks.track_id=:trackId"));
        playCounterQuery.bindValue(
                QStringLiteral(":playlistHidden"),
                QVariant(PlaylistDAO::PLHT_SET_LOG));
        auto trackUpdateQuery = FwdSqlQuery(
                m_database,
                QStringLiteral(
                        "UPDATE library SET "
                        "timesplayed=:timesplayed,"
                        "last_played_at=:last_played_at "
                        "WHERE library.id=:trackId"));
        for (const auto& trackId : trackIds) {
            playCounterQuery.bindValue(
                    QStringLiteral(":trackId"),
                    trackId);
            VERIFY_OR_DEBUG_ASSERT(!playCounterQuery.hasError()) {
                continue;
            }
            VERIFY_OR_DEBUG_ASSERT(playCounterQuery.execPrepared()) {
                continue;
            }
            QVariant timesplayed;
            QVariant last_played_at;
            DEBUG_ASSERT(last_played_at.isNull());
            if (playCounterQuery.next()) {
                timesplayed = playCounterQuery.fieldValue(0);
                last_played_at = playCounterQuery.fieldValue(1);
                // Result is a single row
                DEBUG_ASSERT(!playCounterQuery.next());
            }
            if (timesplayed.isNull()) {
                // Never played and timesplayed should not be NULL
                DEBUG_ASSERT(last_played_at.isNull());
                timesplayed = 0;
            }
            trackUpdateQuery.bindValue(
                    QStringLiteral(":trackId"),
                    trackId);
            trackUpdateQuery.bindValue(
                    QStringLiteral(":timesplayed"),
                    timesplayed);
            trackUpdateQuery.bindValue(
                    QStringLiteral(":last_played_at"),
                    last_played_at);
            VERIFY_OR_DEBUG_ASSERT(!trackUpdateQuery.hasError()) {
                continue;
            }
            VERIFY_OR_DEBUG_ASSERT(trackUpdateQuery.execPrepared()) {
                continue;
            }
            // 0 for tracks that have just been deleted
            DEBUG_ASSERT(trackUpdateQuery.numRowsAffected() <= 1);
        }
    }
#endif // __SQLITE3__
    // TODO: DAOs should be passive and simply execute queries. They
    // should neither make assumptions about transaction boundaries
    // nor receive or emit any signals.
    emit mixxx::thisAsNonConst(this)->tracksChanged(trackIds);
    return true;
}

//static
void TrackDAO::setTrackGenreInternal(Track* pTrack, const QString& genre) {
    DEBUG_ASSERT(pTrack);
    pTrack->setGenreFromTrackDAO(genre);
}

//static
void TrackDAO::setTrackHeaderParsedInternal(Track* pTrack, bool headerParsed) {
    DEBUG_ASSERT(pTrack);
    pTrack->setHeaderParsedFromTrackDAO(headerParsed);
}

//static
bool TrackDAO::getTrackHeaderParsedInternal(const mixxx::TrackRecord& trackRecord) {
    return trackRecord.m_headerParsed;
}
