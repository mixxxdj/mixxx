#include "library/vdj/vdjfeature.h"

#include <QCryptographicHash>
#include <QFileInfo>
#include <QMap>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSettings>
#include <QStandardPaths>
#include <QXmlStreamReader>
#include <QDebug>
#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/missing_hidden/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_vdjfeature.cpp"
#include "track/beats.h"
#include "track/cue.h"
#include "track/keyutils.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/sandbox.h"
#include "util/semanticversion.h"
#include "vdj_parser_factory.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

#define IS_VDJ_DEVICE "::isVdjDevice::"
#define IS_NOT_VDJ_DEVICE "::isNotVdjDevice::"

namespace {
const QString kVdjDbPath = QStringLiteral("VirtualDJ/database.xml");
const QString kVdjDbv6Path = QStringLiteral("VirtualDJ Local Database v6.xml");
const QString kVdjDbv6InUserFolderPath = QStringLiteral("VirtualDJ/VirtualDJ Database v6.xml");
const QString kVdjLibraryTable = QStringLiteral("vdj_library");
const QString kVdjPlaylistsTable = QStringLiteral("vdj_playlists");
const QString kVdjPlaylistTracksTable = QStringLiteral("vdj_playlist_tracks");
const QString kVdjTrackCuesTable = QStringLiteral("vdj_cues");

struct vdj_cue_t {
    QVariant startPosition;
    QVariant endPosition;
    QString posType;
    bool isHotCue;
    bool isLoopCue;
    QVariant hotCueIndex;
    QVariant cueColor;
};

// ---------- PATH NORMALIZATION HELPERS ----------

static bool isExternalDeviceBase(const QString& base) {
    // Heuristics: external/volume-like base paths
    const QString b = QDir::fromNativeSeparators(base);
    // Windows root "X:/"
    if (b.size() >= 3 && b[1] == QLatin1Char(':') && b.endsWith(QLatin1Char('/')))
        return true;
    // macOS volumes
    if (b.startsWith(QStringLiteral("/Volumes/")))
        return true;
    // Linux mounts
    if (b.startsWith(QStringLiteral("/media/")) || b.startsWith(QStringLiteral("/run/media/")))
        return true;
    return false;
}

static bool isUNCPath(const QString& p) {
    // Windows UNC: \\server\share\...
    return p.startsWith(QStringLiteral("\\\\")) || p.startsWith(QStringLiteral("//"));
}

static QString toNativeClean(const QString& p) {
    // Normalize slashes and clean redundant segments
    return QDir::cleanPath(QDir::fromNativeSeparators(p));
}

// Convert file:// URLs to local paths if needed
static QString urlToLocalIfPossible(const QString& p) {
    if (p.startsWith(QStringLiteral("file://"), Qt::CaseInsensitive)) {
        const QUrl u(p);
        if (u.isLocalFile())
            return u.toLocalFile();
    }
    return p;
}

// Strip well-known absolute roots (drive letter, /Volumes/<label>, /media/<user>/<label>, /run/media/<user>/<label>)
// Return a *device-relative* path like "Music/Artist/Track.mp3".
static QString relativizeAgainstKnownRoots(QString raw) {
    raw = urlToLocalIfPossible(raw);
    QString p = QDir::fromNativeSeparators(raw);

    // 1) Windows drive "X:/..."
    {
        static const QRegularExpression rx(R"(^[A-Za-z]:/(.*)$)");
        auto m = rx.match(p);
        if (m.hasMatch())
            return m.captured(1);
    }
    // 2) macOS "/Volumes/<label>/..."
    {
        static const QRegularExpression rx(R"(^/Volumes/[^/]+/(.*)$)");
        auto m = rx.match(p);
        if (m.hasMatch())
            return m.captured(1);
    }
    // 3) Linux "/media/<user>/<label>/..."
    {
        static const QRegularExpression rx(R"(^/media/[^/]+/[^/]+/(.*)$)");
        auto m = rx.match(p);
        if (m.hasMatch())
            return m.captured(1);
    }
    // 4) Linux "/run/media/<user>/<label>/..."
    {
        static const QRegularExpression rx(R"(^/run/media/[^/]+/[^/]+/(.*)$)");
        auto m = rx.match(p);
        if (m.hasMatch())
            return m.captured(1);
    }

    // 5) If it starts with a leading slash (unlikely on device DB), make it relative anyway
    while (p.startsWith(QLatin1Char('/')))
        p.remove(0, 1);
    return p;
}

// Build an absolute path suitable for Mixxx 'location'
// - For *external device DBs*: force device-relative remap => deviceBasePath + relative
// - For *master DBs* (Documents/App Support): keep as-is (normalized), unless it looks
//   like a Windows/mac mount path from another OS, in which case we still remap.
static QString materializePathForDevice(const QString& rawPath,
        const QString& deviceBasePath,
        bool deviceIsExternalLike) {
    if (rawPath.trimmed().isEmpty())
        return rawPath;

    // UNC shares: leave unchanged (we don't want to remap network locations).
    const QString raw = urlToLocalIfPossible(rawPath);
    if (isUNCPath(raw))
        return toNativeClean(raw);

    if (deviceIsExternalLike) {
        // External DB: always remap to current mountpoint.
        QString rel = relativizeAgainstKnownRoots(raw);
        // Drop any leading slashes before joining
        while (rel.startsWith(QLatin1Char('/')))
            rel.remove(0, 1);
        const QString abs = QDir(deviceBasePath).filePath(rel);
        return toNativeClean(abs);
    }

    // Master DB: keep absolute paths. However, if the path *looks* like a foreign
    // absolute (e.g. "E:/…", "/Volumes/USB/…"), still remap to the user's deviceBasePath.
    // This helps when a Windows DB is copied to macOS, etc.
    const QString normalized = toNativeClean(raw);
    const bool looksForeignAbs =
            normalized.contains(QRegularExpression(R"(^[A-Za-z]:/)")) ||
            normalized.startsWith(QStringLiteral("/Volumes/")) ||
            normalized.startsWith(QStringLiteral("/media/")) ||
            normalized.startsWith(QStringLiteral("/run/media/"));

    if (looksForeignAbs) {
        QString rel = relativizeAgainstKnownRoots(normalized);
        while (rel.startsWith(QLatin1Char('/')))
            rel.remove(0, 1);
        const QString abs = QDir(deviceBasePath).filePath(rel);
        return toNativeClean(abs);
    }

    return normalized;
}

// ----------------------------------------------------------------

static QString pathHash(const QString& canonPath) {
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(canonPath.toUtf8());
    return QString::fromLatin1(h.result().toHex());
}

static QString deviceIdForPath(const QString& canonPath) {
    // Optional: volume/UUID in case we would like to bind this 
    // At the moment, ignore and just use location
    return QString();
}

static mixxx::BeatsPointer makeConstantGrid(
        double sampleRate, double bpm, mixxx::audio::FramePos firstBeat) {
    return mixxx::Beats::fromConstTempo(mixxx::audio::SampleRate(sampleRate), firstBeat, mixxx::Bpm(bpm));
}

static inline mixxx::RgbColor::optional_t vdjColorToMixxx(std::optional<int> rgbMaybe) {
    if (!rgbMaybe)
        return std::nullopt;
    const uint32_t c = static_cast<uint32_t>(*rgbMaybe);
    const uint8_t r = (c >> 16) & 0xFF;
    const uint8_t g = (c >> 8) & 0xFF;
    const uint8_t b = (c) & 0xFF;

    return mixxx::RgbColor(qRgb(r, g, b));
}

static inline mixxx::audio::FramePos vdjPosToFrames(
        double value, vdj::PoiUnit unit, double bpmForBeats, double sampleRate) {
    switch (unit) {
    case vdj::PoiUnit::Millis:
        return mixxx::audio::FramePos::fromEngineSamplePos(value * sampleRate / 1000.0);
    case vdj::PoiUnit::Samples441:
        return mixxx::audio::FramePos::fromEngineSamplePos(value * (sampleRate / 44100.0));
    case vdj::PoiUnit::Beats: {
        const double bpm = bpmForBeats > 0.0 ? bpmForBeats : 120.0; // fallback
        const double seconds = (value * 60.0) / bpm;
        return mixxx::audio::FramePos::fromEngineSamplePos(seconds * sampleRate);
    }
    }
    return mixxx::audio::FramePos::fromEngineSamplePos(0.0);
}


CuePointer makeLoopCue(
        TrackPointer pTrack,
        mixxx::audio::FramePos startF,
        mixxx::audio::FramePos endF,
        const QString& label,
        mixxx::RgbColor::optional_t color) {
    
    CuePointer pCue;
    pCue = pTrack->createAndAddCue(
            mixxx::CueType::Loop,
            -1,
            startF,
            endF);

    pCue->setLabel(label);
        
    if (color) {
        pCue->setColor(*color);
    }
    return pCue;
}

CuePointer makeHotCue(
        TrackPointer pTrack,
        mixxx::audio::FramePos posF,
        int hotIndex, // -1 for unnumbered
        const QString& label,
        mixxx::RgbColor::optional_t color) {
    CuePointer pCue;
    const QList<CuePointer> cuePoints = pTrack->getCuePoints();
    for (const CuePointer& trackCue : cuePoints) {
        if (trackCue->getHotCue() == hotIndex) {
            pCue = trackCue;
            break;
        }
    }

    mixxx::CueType type = mixxx::CueType::HotCue;
    
    if (pCue) {
        pCue->setStartPosition(posF);
    } else {
        pCue = pTrack->createAndAddCue(
                type,
                hotIndex,
                posF,
                posF
                );
    }
    pCue->setLabel(label);
    if (color) {
        pCue->setColor(*color);
    }

    return pCue;
}


// ---------------------------------------------------------

bool createLibraryTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Virtual DJ library table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    vdj_id INTEGER,"
            "    artist TEXT,"
            "    title TEXT,"
            "    album TEXT,"
            "    year INTEGER,"
            "    genre TEXT,"
            "    tracknumber TEXT,"
            "    location TEXT UNIQUE,"
            "    comment TEXT,"
            "    duration INTEGER,"
            "    bitrate TEXT,"
            "    bpm FLOAT,"
            "    key TEXT,"
            "    rating INTEGER,"
            "    analyze_path TEXT UNIQUE,"
            "    device TEXT,"
            "    color INTEGER"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool createTrackCuesTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Virtual DJ track cues table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            "(" 
            "   id INTEGER PRIMARY KEY AUTOINCREMENT," 
            "   device_id TEXT," 
            "   location TEXT," 
            "   location_hash TEXT," 
            "   pos_type TEXT," 
            "   position DOUBLE," 
            "   endpos DOUBLE," 
            "   hotcue_index INTEGER," 
            "   is_hotcue INTEGER," 
            "   is_loop INTEGER," 
            "   color INTEGER" 
            ");" 
            /*""
            "CREATE INDEX IF NOT EXISTS idx_vdj_cues_loc ON vdj_cues(location);" 
            "CREATE INDEX IF NOT EXISTS idx_vdj_cues_dev_loc ON vdj_cues(device_id, location);" 
            "CREATE INDEX IF NOT EXISTS idx_vdj_cues_hash ON vdj_cues(location_hash);"*/
    );
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}


bool createPlaylistsTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Virtual DJ playlists table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY,"
            "    name TEXT UNIQUE"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool createPlaylistTracksTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Virtual DJ playlist tracks table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    playlist_id INTEGER REFERENCES vdj_playlists(id),"
            "    track_id INTEGER REFERENCES vdj_library(id),"
            "    position INTEGER"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

void clearDeviceTables(QSqlDatabase& database, TreeItem* child) {
    ScopedTransaction transaction(database);

    int trackID = -1;
    int playlistID = kInvalidPlaylistId;
    QSqlQuery tracksQuery(database);
    tracksQuery.prepare("select id from " + kVdjLibraryTable + " where device=:device");
    tracksQuery.bindValue(":device", child->getLabel());

    QSqlQuery deleteCuesQuery(database);
    deleteCuesQuery.prepare("delete from " + kVdjTrackCuesTable + " where track_id=:id");

    QSqlQuery deletePlaylistsQuery(database);
    deletePlaylistsQuery.prepare("delete from " + kVdjPlaylistsTable + " where id=:id");

    QSqlQuery deletePlaylistTracksQuery(database);
    deletePlaylistTracksQuery.prepare("delete from " +
            kVdjPlaylistTracksTable + " where playlist_id=:playlist_id");

    if (!tracksQuery.exec()) {
        LOG_FAILED_QUERY(tracksQuery)
                << "device:" << child->getLabel();
    }

    while (tracksQuery.next()) {
        trackID = tracksQuery.value(tracksQuery.record().indexOf("id")).toInt();

        deleteCuesQuery.bindValue(":track_id", trackID);
        if (!deleteCuesQuery.exec()) {
            LOG_FAILED_QUERY(deleteCuesQuery)
                    << "trackID:" << trackID;
        }

        QSqlQuery playlistTracksQuery(database);
        playlistTracksQuery.prepare("select playlist_id from " +
                kVdjPlaylistTracksTable + " where track_id=:track_id");
        playlistTracksQuery.bindValue(":track_id", trackID);

        if (!playlistTracksQuery.exec()) {
            LOG_FAILED_QUERY(playlistTracksQuery)
                    << "trackID:" << trackID;
        }

        while (playlistTracksQuery.next()) {
            playlistID = playlistTracksQuery
                                 .value(playlistTracksQuery.record().indexOf(
                                         "playlist_id"))
                                 .toInt();

            deletePlaylistsQuery.bindValue(":id", playlistID);

            if (!deletePlaylistsQuery.exec()) {
                LOG_FAILED_QUERY(deletePlaylistsQuery)
                        << "playlistId:" << playlistID;
            }

            deletePlaylistTracksQuery.bindValue(":playlist_id", playlistID);

            if (!deletePlaylistTracksQuery.exec()) {
                LOG_FAILED_QUERY(deletePlaylistTracksQuery)
                        << "playlistId:" << playlistID;
            }
        }
    }

    QSqlQuery deleteTracksQuery(database);
    deleteTracksQuery.prepare("delete from " + kVdjLibraryTable + " where device=:device");
    deleteTracksQuery.bindValue(":device", child->getLabel());

    if (!deleteTracksQuery.exec()) {
        LOG_FAILED_QUERY(deleteTracksQuery)
                << "device:" << child->getLabel();
    }

    transaction.commit();
}

bool dropTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Dropping Virtual DJ table: " << tableName;

    QSqlQuery query(database);
    query.prepare("DROP TABLE IF EXISTS " + tableName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}
TreeItem* findVdjMasterDeviceLocal() {
    QString docs = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString home = QDir::homePath();

#if defined(__WINDOWS__)
    // Documents\VirtualDJ\database.xml
    const mixxx::FileInfo modernWin(docs + QLatin1Char('/') + kVdjDbPath);
    const mixxx::FileInfo v6Win(docs + QLatin1Char('/') + kVdjDbv6InUserFolderPath);

    if ((modernWin.exists() && modernWin.isFile()) ||
            (v6Win.exists() && v6Win.isFile())) {
        QList<QString> data{docs, IS_VDJ_DEVICE};
        return new TreeItem(QStringLiteral("This Computer"), QVariant(data));
    }
#elif defined(__APPLE__)
    // 1) Newer builds: ~/Library/Application Support/VirtualDJ/database.xml
    const mixxx::FileInfo modernAppSup(home + QStringLiteral("/Library/Application Support/") + kVdjDbPath.mid(10));

    // 2) Older/imported: ~/Documents/VirtualDJ/database.xml
    const mixxx::FileInfo modernDocs(docs + QLatin1Char('/') + kVdjDbPath);

    // 3) V6 in Documents
    const mixxx::FileInfo v6Docs(docs + QLatin1Char('/') + kVdjDbv6InUserFolder);

    if ((modernAppSup.exists() && modernAppSup.isFile()) ||
            (modernDocs.exists() && modernDocs.isFile()) ||
            (v6Docs.exists() && v6Docs.isFile())) {
        // Choose “Documents” as base if exists, otherwise "Application Support"
        QString base = docs;
        if (!(modernDocs.exists() || v6Docs.exists())) {
            base = home + QStringLiteral("/Library/Application Support");
        }
        QList<QString> data{base, IS_VDJ_DEVICE};
        return new TreeItem(QStringLiteral("This Mac"), QVariant(data));
    }
#else
    // THERE IS NO INSTALLED VDJ ON LINUX - SKIP
#endif

    return nullptr;
}

// This function is executed in a separate thread other than the main thread
// The returned list owns the pointers, but we can't use a unique_ptr because
// the result is passed by a const reference inside QFuture and than copied
// to the main thread requiring a copy-able object.
QList<TreeItem*> findVdjDevices() {
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    QList<TreeItem*> foundDevices;

    // STEPS:
    //
    // 1) Lookup main virtual deejay database
    // 2) Lookup external databases
    //

    // STEP 1: MAIN database
    if (TreeItem* local = findVdjMasterDeviceLocal()) {
        foundDevices << local;
    }

    // STEP 2: External databases

#if defined(__WINDOWS__)
    // Repopulate drive list
    QFileInfoList drives = QDir::drives();
    // show drive letters
    foreach (QFileInfo drive, drives) {
        // Using drive.filePath() instead of drive.canonicalPath() as it
        // freezes interface too much if there is a network share mounted
        // (drive letter assigned) but unavailable
        //
        // drive.canonicalPath() make a system call to the underlying filesystem
        // introducing delay if it is unreadable.
        // drive.filePath() doesn't make any access to the filesystem and consequently
        // shorten the delay

        mixxx::FileInfo modernVdjDBFileInfo(drive.filePath() + kVdjDbPath);
        mixxx::FileInfo v6VdjDBFileInfo(drive.filePath() + kVdjDbv6Path);

        qDebug() << modernVdjDBFileInfo.location();
        qDebug() << v6VdjDBFileInfo.location();

        if ((modernVdjDBFileInfo.exists() && modernVdjDBFileInfo.isFile()) ||
                (v6VdjDBFileInfo.exists() && v6VdjDBFileInfo.isFile())) {
            QString displayPath = drive.filePath();
            if (displayPath.endsWith("/")) {
                displayPath.chop(1);
            }
            QList<QString> data;
            data << drive.filePath();
            data << IS_VDJ_DEVICE;
            auto* pFoundDevice = new TreeItem(
                    std::move(displayPath),
                    QVariant(data));
            foundDevices << pFoundDevice;
        }
    }
#elif defined(__LINUX__)
    // To get devices on Linux, we look for directories under /media and
    // /run/media/$USER.
    QFileInfoList devices;

    // Add folders under /media to devices.
    devices += QDir(QStringLiteral("/media")).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /media/$USER to devices.
    QDir mediaUserDir(QStringLiteral("/media/") + QString::fromLocal8Bit(qgetenv("USER")));
    devices += mediaUserDir.entryInfoList(
            QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /run/media/$USER to devices.
    QDir runMediaUserDir(QStringLiteral("/run/media/") + QString::fromLocal8Bit(qgetenv("USER")));
    devices += runMediaUserDir.entryInfoList(
            QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach (QFileInfo device, devices) {
        mixxx::FileInfo modernVdjDBFileInfo(device.filePath() + QStringLiteral("/") + kVdjDbPath);
        mixxx::FileInfo v6VdjDBFileInfo(device.filePath() + QStringLiteral("/") + kVdjDbv6Path);
        if ((modernVdjDBFileInfo.exists() && modernVdjDBFileInfo.isFile()) ||
                (v6VdjDBFileInfo.exists() && v6VdjDBFileInfo.isFile())) {
            auto* pFoundDevice = new TreeItem(
                    device.fileName(),
                    QVariant(QList<QString>{device.filePath(), IS_VDJ_DEVICE}));
            foundDevices << pFoundDevice;
        }
    }
#else // MACOS
    QFileInfoList devices = QDir(QStringLiteral("/Volumes")).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach (QFileInfo device, devices) {
        mixxx::FileInfo modernVdjDBFileInfo(device.filePath() + QStringLiteral("/") + kVdjDbPath);
        mixxx::FileInfo v6VdjDBFileInfo(device.filePath() + QStringLiteral("/") + kVdjDbv6Path);

        if ((modernVdjDBFileInfo.exists() && modernVdjDBFileInfo.isFile()) ||
                (v6VdjDBFileInfo.exists() && v6VdjDBFileInfo.isFile())) {
            QList<QString> data;
            data << device.filePath();
            data << IS_VDJ_DEVICE;
            auto* pFoundDevice = new TreeItem(
                    device.fileName(),
                    QVariant(data));
            foundDevices << pFoundDevice;
        }
    }
#endif

    return foundDevices;
}

int createDevicePlaylist(QSqlDatabase& database, const QString& devicePath) {
    int playlistId = kInvalidPlaylistId;

    QSqlQuery queryInsertIntoDevicePlaylist(database);
    queryInsertIntoDevicePlaylist.prepare(
            "INSERT INTO " + kVdjPlaylistsTable +
            " (name) "
            "VALUES (:name)");

    queryInsertIntoDevicePlaylist.bindValue(":name", devicePath);

    if (!queryInsertIntoDevicePlaylist.exec()) {
        LOG_FAILED_QUERY(queryInsertIntoDevicePlaylist)
                << "devicePath: " << devicePath;
        return playlistId;
    }

    QSqlQuery idQuery(database);
    idQuery.prepare("select id from " + kVdjPlaylistsTable + " where name=:path");
    idQuery.bindValue(":path", devicePath);

    if (!idQuery.exec()) {
        LOG_FAILED_QUERY(idQuery)
                << "devicePath: " << devicePath;
        return playlistId;
    }

    while (idQuery.next()) {
        playlistId = idQuery.value(idQuery.record().indexOf("id")).toInt();
    }

    return playlistId;
}



bool insertTrack(QSqlDatabase& database,
        vdj::Track track,
        QSqlQuery& insertIntoLibraryQuery,
        QSqlQuery& insertIntoDevicePlaylistTracksQuery,
        QSqlQuery& insertIntoTrackCueTableQuery,
        const QString& deviceLabel,
        const QString& deviceBasePath,
        const bool deviceIsExternalLike,
        int fileNumber) {
    
    // Basic fields (QStrings are fine if empty)
    const QString artist = track.artist;
    const QString title = track.title;
    const QString album = track.album;
    const QString genre = track.genre;
    const QString comment = track.comment;
    const QString key = track.key;

    // Year in DB is INTEGER; try to parse t.year which may be string like "2020" or empty
    QVariant yearVar = QVariant(); // NULL by default
    if (!track.year.isEmpty()) {
        bool ok = false;
        const int y = track.year.toInt(&ok);
        if (ok)
            yearVar = y;
    }

    // Track number: we don't parse it (VDJ rarely stores it here) - leave NULL/empty
    const QString tracknumber;

    // Location - Transform to a location we can actually open on this machine
    const QString location =
            materializePathForDevice(track.filePath, deviceBasePath, deviceIsExternalLike);

    // Hash for lookup fallback
    QString locationHash = pathHash(location);

    // Duration: vdj::Track stores optional lengthSeconds (double). Store integer seconds.
    QVariant durationVar = QVariant();
    if (track.lengthSeconds) {
        // Mixxx library convention: INTEGER seconds
        durationVar = static_cast<qint64>(std::llround(*track.lengthSeconds));
    }

    // Bitrate: schema says TEXT; we can store "320" or "320 kbps". Use plain number as text.
    QString bitrateText;
    if (track.bitrateKbps) {
        bitrateText = QString::number(*track.bitrateKbps);
    }

    // BPM: double, nullable
    QVariant bpmVar = QVariant();
    if (track.bpm) {
        bpmVar = *track.bpm;
    }

    // Rating: nullable int 0..5 (modern) or whatever VDJ stores; we pass-through if present
    QVariant ratingVar = QVariant();
    if (track.rating)
        ratingVar = *track.rating;

    // Color: optional int
    QVariant colorVar = QVariant();
    if (track.color)
        colorVar = *track.color;

    // vdj_id: not available from our parsers (VDJ doesn't expose a stable numeric ID in DB XML),
    // bind NULL
    insertIntoLibraryQuery.bindValue(":vdj_id", QVariant());

    insertIntoLibraryQuery.bindValue(":artist", artist);
    insertIntoLibraryQuery.bindValue(":title", title);
    insertIntoLibraryQuery.bindValue(":album", album);
    insertIntoLibraryQuery.bindValue(":year", yearVar);
    insertIntoLibraryQuery.bindValue(":genre", genre);
    insertIntoLibraryQuery.bindValue(":tracknumber", tracknumber);
    insertIntoLibraryQuery.bindValue(":location", location);
    insertIntoLibraryQuery.bindValue(":comment", comment);
    insertIntoLibraryQuery.bindValue(":duration", durationVar);
    insertIntoLibraryQuery.bindValue(":bitrate", bitrateText);
    insertIntoLibraryQuery.bindValue(":bpm", bpmVar);
    insertIntoLibraryQuery.bindValue(":key", key);
    insertIntoLibraryQuery.bindValue(":rating", ratingVar);

    // analyze_path: leave NULL
    insertIntoLibraryQuery.bindValue(":analyze_path", QVariant());

    insertIntoLibraryQuery.bindValue(":device", deviceLabel);
    insertIntoLibraryQuery.bindValue(":color", colorVar);

    if (!insertIntoLibraryQuery.exec()) {
        LOG_FAILED_QUERY(insertIntoLibraryQuery) << "[VDJ] insert failed for" << location;
        return false;
    }
    auto trackInsertId = insertIntoLibraryQuery.lastInsertId();
    insertIntoDevicePlaylistTracksQuery.bindValue(":track_id", trackInsertId.toInt());
    insertIntoDevicePlaylistTracksQuery.bindValue(":position", fileNumber);

    if (!insertIntoDevicePlaylistTracksQuery.exec()) {
        LOG_FAILED_QUERY(insertIntoDevicePlaylistTracksQuery) << "[VDJ] insert failed for" << location;
        return false;
    }

    // Cue points to db
    for (auto poi : track.pois) {
        bool isHotCue = false;
        bool isLoopCue = false;
        QVariant startFrame = QVariant(poi.posValue);
        QVariant endFrame = QVariant(poi.posValue + poi.lengthValue);
        QVariant hotCueIdx = QVariant();

        switch (poi.type) {
        case vdj::Poi::PoiType::Loop:
            isHotCue = poi.cueslot.has_value();
            isLoopCue = true;
            break;
        case vdj::Poi::PoiType::Hotcue:
            isHotCue = true;
            isLoopCue = poi.lengthValue > 0;
            if (poi.cueslot.has_value()) {
                hotCueIdx = poi.cueslot.value();
            }
            break;
        default:
            // Skip for now
            continue;
        }
                
        switch (poi.posUnit) {
            case vdj::PoiUnit::Beats:
                insertIntoTrackCueTableQuery.bindValue(":pos_type", "Beats");
            break;
            case vdj::PoiUnit::Millis:
                insertIntoTrackCueTableQuery.bindValue(":pos_type", "Millis");
            break;
            default:
                insertIntoTrackCueTableQuery.bindValue(":pos_type", "Samples441");
            break;
        }
        insertIntoTrackCueTableQuery.bindValue(":dev", deviceIdForPath(location));
        insertIntoTrackCueTableQuery.bindValue(":loc", location);
        insertIntoTrackCueTableQuery.bindValue(":hash", locationHash);
        insertIntoTrackCueTableQuery.bindValue(":pos", startFrame);
        insertIntoTrackCueTableQuery.bindValue(":endpos", endFrame);
        insertIntoTrackCueTableQuery.bindValue(":hotidx", hotCueIdx);
        insertIntoTrackCueTableQuery.bindValue(":is_hot", isHotCue ? 1 : 0);
        insertIntoTrackCueTableQuery.bindValue(":is_loop", isLoopCue ? 1 : 0);
        insertIntoTrackCueTableQuery.bindValue(":color", QVariant());
        
        if (!insertIntoTrackCueTableQuery.exec()) {
            LOG_FAILED_QUERY(insertIntoTrackCueTableQuery) << "[VDJ] cue insert failed for" << location;
        }
    }

    return true;
}

QString parseDeviceDB(mixxx::DbConnectionPoolPtr dbConnectionPool, TreeItem* deviceItem) {
    const QString deviceLabel = deviceItem->getLabel();
    const QList<QVariant> data = deviceItem->getData().toList();
    const QString deviceBasePath = data.isEmpty() ? QString() : data[0].toString();

    qDebug() << "[VDJ] parseDeviceDB() deviceLabel =" << deviceLabel
             << " basePath =" << deviceBasePath;

    // Resolve candidate database paths for modern and v6 layouts
    // Modern (default): <base>/VirtualDJ/database.xml
    const QString modernDbPath = deviceBasePath + QLatin1Char('/') + kVdjDbPath;

    // macOS: some installs use ~/Library/Application Support/VirtualDJ/database.xml
    QString modernDbPathAppSup;
#if defined(__APPLE__)
    if (deviceBasePath.endsWith(QStringLiteral("/Library/Application Support"))) {
        modernDbPathAppSup = deviceBasePath + QStringLiteral("/VirtualDJ/database.xml");
    }
#endif

    // v6 on external root: <base>/VirtualDJ Local Database v6.xml
    const QString v6DbPathRoot = deviceBasePath + QLatin1Char('/') + kVdjDbv6Path;
    // v6 in user Documents: <base>/VirtualDJ/VirtualDJ Database v6.xml
    const QString v6DbPathDocs = deviceBasePath + QLatin1Char('/') + kVdjDbv6InUserFolderPath;

    // Decide which file actually exists
    QString actualPath;

#if defined(__APPLE__)
    bool modernExists = QFile::exists(modernDbPath) || (!modernDbPathAppSup.isEmpty() && QFile::exists(modernDbPathAppSup));
#else
    bool modernExists = QFile::exists(modernDbPath);
#endif

    bool v6Exists = QFile::exists(v6DbPathRoot) || QFile::exists(v6DbPathDocs);

    if (!modernExists && !v6Exists) {
        qWarning() << "[VDJ] No VirtualDJ database found under base path:" << deviceBasePath;
        return deviceBasePath; // nothing to do
    }

#if defined(__APPLE__)
    if (modernExists) {
        actualPath = QFile::exists(modernDbPath) ? modernDbPath : modernDbPathAppSup;
    } else {
        actualPath = QFile::exists(v6DbPathRoot) ? v6DbPathRoot : v6DbPathDocs;
    }
#else
    actualPath = modernExists ? modernDbPath
                              : (QFile::exists(v6DbPathRoot) ? v6DbPathRoot : v6DbPathDocs);
#endif

    qDebug() << "[VDJ] Using database file:" << actualPath;

    // Open file for parsing
    QFile dbFile(actualPath);
    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[VDJ] Failed to open database file:" << actualPath << dbFile.errorString();
        return deviceBasePath;
    }

    // Pick strategy: modern vs v6 based on filename
    const bool isV6 = mixxx::FileInfo(actualPath).fileName().contains(QStringLiteral("v6"), Qt::CaseInsensitive);
    std::unique_ptr<vdj::IDatabaseParser> dbParser =
            vdj::makeParser(&dbFile, isV6 ? vdj::V6 : vdj::Modern);

    vdj::Database parsed;
    QString parseError;
    if (!dbParser || !dbParser->parse(parsed, &parseError)) {
        qWarning() << "[VDJ] Parse failed:" << parseError;
        return deviceBasePath;
    }

    qDebug() << "[VDJ] Parsed" << parsed.tracks.size() << "tracks from" << actualPath
             << "Version:" << parsed.version;

    // DB scope (pooled)
    const mixxx::DbConnectionPooler pooler(dbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(dbConnectionPool);
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "[VDJ] Could not open Mixxx DB connection";
        return deviceBasePath;
    }

    // clean old rows for this device (safe on repeated scans)
    ScopedTransaction cleanupTx(database);
    QSqlQuery deleteFromLibraryQuery(database);
    deleteFromLibraryQuery.prepare("DELETE FROM " + kVdjLibraryTable + " WHERE device=:device");
    deleteFromLibraryQuery.bindValue(":device", deviceLabel);
    if (!deleteFromLibraryQuery.exec()) {
        LOG_FAILED_QUERY(deleteFromLibraryQuery) << "[VDJ] cleanup device =" << deviceLabel;
    }
    cleanupTx.commit();

    // Insert all parsed tracks
    ScopedTransaction tx(database);
    QSqlQuery insertIntoLibraryQuery(database);

    // use INSERT OR IGNORE to skip duplicates gracefully if any.
    insertIntoLibraryQuery.prepare(
            "INSERT OR IGNORE INTO " + kVdjLibraryTable +
            " (vdj_id, artist, title, album, year, genre, tracknumber, location, " +
            "  comment, duration, bitrate, bpm, key, rating, analyze_path, device, color) " +
            "VALUES (:vdj_id, :artist, :title, :album, :year, :genre, :tracknumber, :location, " +
            "        :comment, :duration, :bitrate, :bpm, :key, :rating, :analyze_path, :device, :color)");

    QSqlQuery insertIntoDevicePlaylistTracksQuery(database);
    insertIntoDevicePlaylistTracksQuery.prepare(
            "INSERT INTO " + kVdjPlaylistTracksTable +
            " (playlist_id, track_id, position) " +
            "VALUES (:playlist_id, :track_id, :position)");

    QSqlQuery insertIntoTrackCueTableQuery(database);
    insertIntoTrackCueTableQuery.prepare(
            "INSERT INTO "+ kVdjTrackCuesTable + 
            " (device_id, location, location_hash, pos_type, position, endpos, " +
            "  hotcue_index, is_hotcue, is_loop, color) " +
            "VALUES (:dev, :loc, :hash, :pos_type, :pos, :endpos, :hotidx, :is_hot, :is_loop, :color)"
    );
        

    int inserted = 0;

    // create playlist for the tracks on the device
    int playlistId = createDevicePlaylist(database, deviceBasePath);
    insertIntoDevicePlaylistTracksQuery.bindValue(":playlist_id", playlistId);

    // Decide if this DB is on an external-like base (X:/, /Volumes/..., /media/...)
    const bool deviceIsExternalLike = isExternalDeviceBase(deviceBasePath);

    for (auto& track : parsed.tracks) {
        if (!insertTrack(database,
                    track,
                    insertIntoLibraryQuery,
                    insertIntoDevicePlaylistTracksQuery,
                    insertIntoTrackCueTableQuery,
                    deviceLabel,
                    deviceBasePath,
                    deviceIsExternalLike,
                    (inserted + 1))) {
            qWarning() << "Unable to insert track";
            continue;
        }
        ++inserted;
    }

    tx.commit();
    qDebug() << "[VDJ] Inserted" << inserted << "rows into" << kVdjLibraryTable
             << "for device" << deviceLabel;

    return deviceBasePath;
}
} // namespace

VdjPlaylistModel::VdjPlaylistModel(QObject* pParent,
        TrackCollectionManager* pTrackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(pParent,
                  pTrackCollectionManager,
                  "mixxx.db.model.vdj.playlistmodel",
                  kVdjPlaylistsTable,
                  kVdjPlaylistTracksTable,
                  trackSource) {
    qDebug() << "creating VdjPlaylistModel";
}

TrackPointer VdjPlaylistModel::getTrack(const QModelIndex& index) const {
    qDebug() << "VdjPlaylistModel::getTrack";
    
    
    TrackPointer track = BaseExternalPlaylistModel::getTrack(index);
    
    QString location = track->getLocation();
    
    if (!QFile(location).exists()) {
        return track;
    }

    mixxx::audio::SampleRate sampleRate = track->getSampleRate();
    const double sampleRateKHz = sampleRate / 1000.0;
    
    
    // LOOKUP CUES

    QList<vdj_cue_t> cues;
    
    const QString devId = deviceIdForPath(location);
    const QString locHash = pathHash(location);

    QSqlQuery qDump(m_database);
    qDump.exec("SELECT id, device_id, location, position, endpos FROM "+kVdjTrackCuesTable);
    while (qDump.next()) {
        qDebug() << "[VDJ DUMP] " << qDump.value(0).toLongLong() << qDump.value(1).toString() << qDump.value(2).toString()
                 << qDump.value(3).toInt() << qDump.value(4).toInt();
    }


    // 1) Quickest: match on (device_id, location) if device_id not empty
    QSqlQuery q1(m_database);
    bool hasCues = false;
    q1.prepare(
            "SELECT pos_type, position, endpos, hotcue_index, is_hotcue, is_loop, color FROM " + kVdjTrackCuesTable + 
            " WHERE COALESCE(device_id,'') = COALESCE(:dev,'') AND location = :loc " +
            " ORDER BY position ASC"
    );
    q1.bindValue(":dev", devId);
    q1.bindValue(":loc", location);

    if (!q1.exec()) {
        LOG_FAILED_QUERY(q1);
    } else {
        while (q1.next()) {
            hasCues = true;

            vdj_cue_t c;
            c.posType = q1.value(0).toString();
            c.startPosition = q1.value(1);
            c.endPosition = q1.value(2);
            c.hotCueIndex = q1.value(3);
            c.isHotCue = q1.value(4).toBool();
            c.isLoopCue = q1.value(5).toBool();
            c.cueColor = q1.value(6);

            cues << c;
        } 
    }

    // 2) Fallback to just match location
    if (!hasCues) {
        QSqlQuery q2(m_database);
        q2.prepare(
                "SELECT pos_type, position, endpos, hotcue_index, is_hotcue, is_loop, color FROM " + kVdjTrackCuesTable +
                " WHERE location = :loc " +
                " ORDER BY position_frames ASC");

        q2.bindValue(":loc", location);
        if (q2.exec()) {
            while (q2.next()) {
                hasCues = true;
                
                vdj_cue_t c;
                c.posType = q2.value(0).toString();
                c.startPosition = q2.value(1);
                c.endPosition = q2.value(2);
                c.hotCueIndex = q2.value(3);
                c.isHotCue = q2.value(4).toBool();
                c.isLoopCue = q2.value(5).toBool();
                c.cueColor = q2.value(6);

                cues << c;
            }
        } else {
            LOG_FAILED_QUERY(q2);
        }
    }

    // 3) Last resort: using location hash (symlinks/moves)
    if (!hasCues) {
        QSqlQuery q3(m_database);
        q3.prepare(
                "SELECT pos_type, position, endpos, hotcue_index, is_hotcue, is_loop, color FROM " + kVdjTrackCuesTable +
                " WHERE location_hash = :hash " +
                " ORDER BY position_frames ASC");
        q3.bindValue(":hash", locHash);
        if (q3.exec()) {
            while (q3.next()) {
                hasCues = true;
                
                vdj_cue_t c;
                c.posType = q3.value(0).toString();
                c.startPosition = q3.value(1);
                c.endPosition = q3.value(2);
                c.hotCueIndex = q3.value(3);
                c.isHotCue = q3.value(4).toBool();
                c.isLoopCue = q3.value(5).toBool();
                c.cueColor = q3.value(6);

                cues << c;
            }
        } else {
            LOG_FAILED_QUERY(q3);
        }
    }

    if (hasCues) {
        // we have found vdj cues, now create the mixx cues
        for (vdj_cue_t c : cues) {
            mixxx::audio::FramePos startFrame;
            mixxx::audio::FramePos endFrame = mixxx::audio::kInvalidFramePos;

            double sp = c.startPosition.toDouble();
            double ep = c.endPosition.isNull() ? sp : c.endPosition.toDouble();

            if (c.posType == "Samples441") {
                startFrame = mixxx::audio::FramePos(sampleRateKHz * (sp / 44.1));
                if (c.isLoopCue && (sp < ep)) {
                    endFrame = mixxx::audio::FramePos(sampleRateKHz * (ep / 44.1));
                }
            } else if (c.posType == "Beats") {
                double bpm = track->getBpm();
                if (bpm > 0) {
                    double beatlengthms = 60000 / bpm;

                    startFrame = mixxx::audio::FramePos(sampleRateKHz * (sp * beatlengthms));
                    if (c.isLoopCue && (sp < ep)) {
                        endFrame = mixxx::audio::FramePos(sampleRateKHz * (ep * beatlengthms));
                    }
                }
            } else {
                // milliseconds
                startFrame = mixxx::audio::FramePos(sampleRateKHz * sp);
                if (c.isLoopCue && (sp < ep)) {
                    endFrame = mixxx::audio::FramePos(sampleRateKHz * ep);
                }
            }

            if (c.isHotCue) {
                makeHotCue(track, startFrame, c.hotCueIndex.isNull() ? -1 : c.hotCueIndex.toInt(), QString(), mixxx::RgbColor::nullopt());
            } else if (c.isLoopCue) {
                makeLoopCue(track, startFrame, endFrame, QString(), mixxx::RgbColor::nullopt());
            }
        }

        cues.clear();
    }  

    return track;
}

bool VdjPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

void VdjPlaylistModel::initSortColumnMapping() {
    // Add a bijective mapping between the SortColumnIds and column indices
    for (int i = 0; i < static_cast<int>(TrackModel::SortColumnId::IdMax); ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }

    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Artist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Title)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Album)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::AlbumArtist)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Year)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Genre)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Composer)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Grouping)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::TrackNumber)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::FileType)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::NativeLocation)] =
            fieldIndex(ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Comment)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Duration)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::BitRate)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Bpm)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::ReplayGain)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::DateTimeAdded)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::TimesPlayed)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::LastPlayedAt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_LAST_PLAYED_AT);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Rating)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Key)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Preview)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Color)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::CoverArt)] =
            fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::Position)] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);
    m_columnIndexBySortColumnId[static_cast<int>(
            TrackModel::SortColumnId::PlaylistDateTimeAdded)] =
            fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_DATETIMEADDED);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = static_cast<int>(TrackModel::SortColumnId::IdMin);
            i < static_cast<int>(TrackModel::SortColumnId::IdMax);
            ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(
                m_columnIndexBySortColumnId[static_cast<int>(sortColumn)],
                sortColumn);
    }
}

VdjFeature::VdjFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig, QStringLiteral("vdj")),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_cancelImport(false) {
    QString tableName = kVdjLibraryTable;
    QString idColumn = LIBRARYTABLE_ID;
    QStringList columns = {
            LIBRARYTABLE_ID,
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_ALBUM,
            LIBRARYTABLE_YEAR,
            LIBRARYTABLE_GENRE,
            LIBRARYTABLE_TRACKNUMBER,
            TRACKLOCATIONSTABLE_LOCATION,
            LIBRARYTABLE_COMMENT,
            LIBRARYTABLE_RATING,
            LIBRARYTABLE_DURATION,
            LIBRARYTABLE_BITRATE,
            LIBRARYTABLE_BPM,
            LIBRARYTABLE_KEY,
    };
    QStringList searchColumns = {
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_ALBUM,
            LIBRARYTABLE_GENRE,
            LIBRARYTABLE_TRACKNUMBER,
            TRACKLOCATIONSTABLE_LOCATION,
            LIBRARYTABLE_COMMENT};

    m_trackSource = QSharedPointer<BaseTrackCache>::create(
            pLibrary->trackCollectionManager()->internalCollection(),
            tableName,
            std::move(idColumn),
            std::move(columns),
            std::move(searchColumns),
            false);

    m_pVdjPlaylistModel = make_parented<VdjPlaylistModel>(
            this, pLibrary->trackCollectionManager(), m_trackSource);

    m_isActivated = false;

    m_title = tr("Virtual DJ");

    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);

    // Drop any leftover temporary VDJ database tables if they exist
    dropTable(database, kVdjPlaylistTracksTable);
    dropTable(database, kVdjPlaylistsTable);
    dropTable(database, kVdjTrackCuesTable);
    dropTable(database, kVdjLibraryTable);

    // Create new temporary VDJ database tables
    createLibraryTable(database, kVdjLibraryTable);
    createTrackCuesTable(database, kVdjTrackCuesTable);
    createPlaylistsTable(database, kVdjPlaylistsTable);
    createPlaylistTracksTable(database, kVdjPlaylistTracksTable);

    transaction.commit();

    connect(&m_devicesFutureWatcher,
            &QFutureWatcher<QList<TreeItem*>>::finished,
            this,
            &VdjFeature::onVdjDevicesFound);
    connect(&m_tracksFutureWatcher,
            &QFutureWatcher<QString>::finished,
            this,
            &VdjFeature::onTracksFound);

    // initialize the model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
}

VdjFeature::~VdjFeature() {
    // FREE UP / DISPOSE ALL CLAIMED RESOURCES HERE
    m_devicesFuture.waitForFinished();
    m_tracksFuture.waitForFinished();

    // Drop temporary database tables on shutdown
    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    dropTable(database, kVdjPlaylistTracksTable);
    dropTable(database, kVdjPlaylistsTable);
    dropTable(database, kVdjTrackCuesTable);
    dropTable(database, kVdjLibraryTable);
    transaction.commit();
}

QVariant VdjFeature::title() {
    return m_title;
}

TreeItemModel* VdjFeature::sidebarModel() const {
    return m_pSidebarModel;
}

QString VdjFeature::formatRootViewHtml() const {
    QString title = tr("Virtual DJ");
    QString summary = tr(
            "Reads Virtual DJ XML based databases from both the main install drive "
            "as the databases on external (USB) drives.<br/>"
            "Mixxx can read a database from any device that contains the "
            "database folders (<tt>VirtualDJ</tt>).<br/>"
            "<br/>"
            "The following data is read:");

    QStringList items;

    items
            << tr("Folders")
            << tr("(Hot)cues (partial)")
            << tr("Loops (cue)");

    QString futureFeatures = tr(
            "in the future I'll also try to add the following data:");

    QStringList futureItems;
    futureItems << tr("Beatgrids")
                << tr("Loops")
                << tr("Playlists");

    QString html;
    QString refreshLink = tr("Check for attached Virtual DJ devices (refresh)");
    html.append(QString("<h2>%1</h2>").arg(title));
    html.append(QString("<p>%1</p>").arg(summary));
    html.append(QString("<ul>"));
    for (const auto& item : std::as_const(items)) {
        html.append(QString("<li>%1</li>").arg(item));
    }
    html.append(QString("</ul>"));
    html.append(QString("<p>%1</p>").arg(futureFeatures));
    html.append(QString("<ul>"));
    for (const auto& item : std::as_const(futureItems)) {
        html.append(QString("<li>%1</li>").arg(item));
    }
    html.append(QString("</ul>"));

    // Colorize links in lighter blue, instead of QT default dark blue.
    // Links are still different from regular text, but readable on dark/light backgrounds.
    // https://github.com/mixxxdj/mixxx/issues/9103
    html.append(QString("<a style=\"color:#0496FF;\" href=\"refresh\">%1</a>")
                    .arg(refreshLink));
    return html;
}

void VdjFeature::activate() {
    qDebug() << "VdjFeature::activate()";

    m_devicesFuture = QtConcurrent::run(findVdjDevices);
    m_devicesFutureWatcher.setFuture(m_devicesFuture);
    m_title = tr("(loading) Virtual DJ");
    // calls a slot in the sidebar model such that 'Virtual DJ (Loading)' is displayed.
    emit featureIsLoading(this, true);

    emit enableCoverArtDisplay(false);
    emit switchToView("VDJHOME");
    emit disableSearch();
}

void VdjFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }

    // access underlying TreeItem object
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }

    // TreeItem list data holds 2 values in a QList and have different meanings.
    // If the 2nd QList element IS_VDJ_DEVICE, the 1st element is the
    // filesystem device path, and the parseDeviceDB concurrent thread to parse
    // the Virtual DJ database is initiated. If the 2nd element is
    // IS_NOT_VDJ_DEVICE, the 1st element is the playlist path and it is
    // activated.

    QList<QVariant> data = item->getData().toList();
    QString playlist = data[0].toString();
    bool doParseDeviceDB = data[1].toString() == IS_VDJ_DEVICE;

    qDebug() << "VdjFeature::activateChild " << item->getLabel()
             << " playlist: " << playlist << " doParseDeviceDB: " << doParseDeviceDB;

    if (doParseDeviceDB) {
        qDebug() << "Parse Virtual DJ Device DB: " << playlist;

        // Let a worker thread do the XML parsing
        m_tracksFuture = QtConcurrent::run(parseDeviceDB, static_cast<Library*>(parent())->dbConnectionPool(), item);
        m_tracksFutureWatcher.setFuture(m_tracksFuture);

        // This device is now a playlist element, future activations should treat is
        // as such
        data[1] = QVariant(IS_NOT_VDJ_DEVICE);
        item->setData(QVariant(data));
    } else {
        qDebug() << "Activate Virtual DJ Playlist: " << playlist;
        m_pVdjPlaylistModel->setPlaylist(playlist);
        emit showTrackModel(m_pVdjPlaylistModel);
    }
}

void VdjFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
        KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    parented_ptr<WLibraryTextBrowser> pEdit = make_parented<WLibraryTextBrowser>(pLibraryWidget);
    pEdit->setHtml(formatRootViewHtml());
    pEdit->setOpenLinks(false);
    connect(pEdit, &WLibraryTextBrowser::anchorClicked, this, &VdjFeature::htmlLinkClicked);
    pLibraryWidget->registerView("VDJHOME", pEdit);
}

bool VdjFeature::isSupported() {
    return true;
}

void VdjFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
}

std::unique_ptr<BaseSqlTableModel> VdjFeature::createPlaylistModelForPlaylist(
        const QString& playlist) {
    auto pModel = std::make_unique<VdjPlaylistModel>(
            this, m_pLibrary->trackCollectionManager(), m_trackSource);
    pModel->setPlaylist(playlist);
    return pModel;
}

void VdjFeature::onVdjDevicesFound() {
    const QList<TreeItem*> result = m_devicesFuture.result();
    auto foundDevices = std::vector<std::unique_ptr<TreeItem>>(result.cbegin(), result.cend());

    clearLastRightClickedIndex();

    TreeItem* root = m_pSidebarModel->getRootItem();
    QSqlDatabase database = m_pTrackCollection->database();

    if (foundDevices.size() == 0) {
        // No VDJ devices found
        ScopedTransaction transaction(database);

        dropTable(database, kVdjPlaylistTracksTable);
        dropTable(database, kVdjPlaylistsTable);
        dropTable(database, kVdjLibraryTable);

        // Create new temporary database tables
        createLibraryTable(database, kVdjLibraryTable);
        createPlaylistsTable(database, kVdjPlaylistsTable);
        createPlaylistTracksTable(database, kVdjPlaylistTracksTable);

        transaction.commit();

        if (root->childRows() > 0) {
            // Devices have since been unmounted
            m_pSidebarModel->removeRows(0, root->childRows());
        }
    } else {
        // We have found VDJ Devices
        for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
            TreeItem* child = root->child(deviceIndex);
            bool removeChild = true;

            for (const auto& pDeviceFound : foundDevices) {
                if (pDeviceFound->getLabel() == child->getLabel()) {
                    removeChild = false;
                    break;
                }
            }

            if (removeChild) {
                // Device has since been unmounted, cleanup DB
                clearDeviceTables(database, child);

                m_pSidebarModel->removeRows(deviceIndex, 1);
            }
        }

        std::vector<std::unique_ptr<TreeItem>> childrenToAdd;

        for (auto&& pDeviceFound : foundDevices) {
            bool addNewChild = true;
            for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
                TreeItem* child = root->child(deviceIndex);

                if (pDeviceFound->getLabel() == child->getLabel()) {
                    // This device already exists in the TreeModel, don't add or parse it again
                    addNewChild = false;
                }
            }

            if (addNewChild) {
                childrenToAdd.push_back(std::move(pDeviceFound));
            }
        }

        if (!childrenToAdd.empty()) {
            m_pSidebarModel->insertTreeItemRows(std::move(childrenToAdd), 0);
        }
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Virtual DJ");
    emit featureLoadingFinished(this);
}

void VdjFeature::onTracksFound() {
    qDebug() << "onTracksFound";
    m_pSidebarModel->triggerRepaint();

    QString devicePlaylist;
    try {
        devicePlaylist = m_tracksFuture.result();
    } catch (const std::exception& e) {
        qWarning() << "Failed to load Virtual DJ database:" << e.what();
        return;
    }

    qDebug() << "Show Virtual DJ Device Playlist: " << devicePlaylist;

    m_pVdjPlaylistModel->setPlaylist(devicePlaylist);
    emit showTrackModel(m_pVdjPlaylistModel);
}

void VdjFeature::refreshLibraryModels() {
}