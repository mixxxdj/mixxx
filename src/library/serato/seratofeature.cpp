// seratofeature.cpp
// Created 2020-01-31 by Jan Holthuis

#include "library/serato/seratofeature.h"

#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>

#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "track/beatfactory.h"
#include "track/cue.h"
#include "track/keyfactory.h"
#include "util/assert.h"
#include "util/color/color.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

// Serato Database Field IDs
// The "magic" value is the short 4 byte ascii code interpreted as quint32, so
// that we can use the value in a switch statement instead of going through
// a strcmp if/else ladder.
enum class FieldId : quint32 {
    Version = 0x7672736e,        // vrsn
    Track = 0x6f74726b,          // otrk
    FileType = 0x74747970,       // ttyp
    FilePath = 0x7066696c,       // pfil
    SongTitle = 0x74736e67,      // tsng
    Artist = 0x74617274,         // tart
    Album = 0x74616c62,          // talb
    Genre = 0x7467656e,          // tgen
    Comment = 0x74636f6d,        // tcom
    Grouping = 0x74677270,       // tgrp
    Label = 0x746c626c,          // tlbl
    Year = 0x74747972,           // ttyr
    Length = 0x746c656e,         // tlen
    Bitrate = 0x74626974,        // tbit
    SampleRate = 0x74736d70,     // tsmp
    Bpm = 0x7462706d,            // tbpm
    DateAddedText = 0x74616464,  // tadd
    DateAdded = 0x75616464,      // uadd
    Key = 0x746b6579,            // tkey
    BeatgridLocked = 0x6262676c, // bbgl
    FileTime = 0x75746d65,       // utme
    Missing = 0x626d6973,        // bmis
    Sorting = 0x7472736f,        // osrt
    ReverseOrder = 0x62726576,   // brev
    ColumnTitle = 0x6f766374,    // ovct
    ColumnName = 0x7476636e,     // tvcn
    ColumnWidth = 0x74766377,    // tvcw
    TrackPath = 0x7074726b,      // ptrk
};

struct serato_track_t {
    QString filetype;
    QString location;
    QString title;
    QString artist;
    QString album;
    QString genre;
    QString comment;
    QString grouping;
    QString label;
    int year = -1;
    int duration = -1;
    QString bitrate;
    QString samplerate;
    double bpm = -1.0;
    QString key;
    bool beatgridlocked = false;
    bool missing = false;
    quint32 filetime = 0;
    quint32 datetimeadded = 0;
};

const QString kDatabaseDirectory = QStringLiteral("_Serato_");
const QString kDatabaseFilename = QStringLiteral("database V2");
const QString kCrateDirectory = QStringLiteral("Subcrates");
const QString kCrateFilter = QStringLiteral("*.crate");
const QString kSmartCrateDirectory = QStringLiteral("Smart Crates");
const QString kSmartCrateFilter = QStringLiteral("*.scrate");

const QString kSeratoLibraryTable = QStringLiteral("serato_library");
const QString kSeratoPlaylistsTable = QStringLiteral("serato_playlists");
const QString kSeratoPlaylistTracksTable = QStringLiteral("serato_playlist_tracks");

constexpr int kHeaderSize = 2 * sizeof(quint32);

int createPlaylist(const QSqlDatabase& database, const QString& name, const QString& databasePath) {
    QSqlQuery query(database);
    query.prepare(
            "INSERT INTO serato_playlists (name, serato_db)"
            "VALUES (:name, :serato_db)");
    query.bindValue(":name", name);
    query.bindValue(":serato_db", databasePath);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "databasePath: " << databasePath;
        return -1;
    }

    return query.lastInsertId().toInt();
}

int insertTrackIntoPlaylist(const QSqlDatabase& database, int playlistId, int trackId, int position) {
    QSqlQuery query(database);
    query.prepare(
            "INSERT INTO serato_playlist_tracks (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");
    query.bindValue(":playlist_id", playlistId);
    query.bindValue(":track_id", trackId);
    query.bindValue(":position", position);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return -1;
    }

    return query.lastInsertId().toInt();
}

inline QString utf16beToQString(const QByteArray& data, const quint32 size) {
    return QTextCodec::codecForName("UTF-16BE")->toUnicode(data, size);
}

inline bool bytesToBoolean(const QByteArray& data) {
    VERIFY_OR_DEBUG_ASSERT(!data.isEmpty()) {
        return false;
    }
    return data.at(0) != 0;
}

inline quint32 bytesToUInt32(const QByteArray& data) {
    VERIFY_OR_DEBUG_ASSERT(data.size() >= static_cast<int>(sizeof(quint32))) {
        return 0;
    }
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return qFromBigEndian<quint32>(data.constData());
#else
    return qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.constData()));
#endif
}

inline bool parseTrack(serato_track_t* track, QIODevice* buffer) {
    QByteArray headerData = buffer->read(kHeaderSize);
    while (headerData.length() == kHeaderSize) {
        quint32 fieldId = bytesToUInt32(headerData.mid(0, sizeof(quint32)));
        quint32 fieldSize = bytesToUInt32(headerData.mid(sizeof(quint32), kHeaderSize));

        // Read field data
        QByteArray data = buffer->read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qWarning() << "Failed to read "
                       << fieldSize
                       << " bytes for "
                       << fieldName
                       << " field.";
            return false;
        }

        // Parse field data
        switch (static_cast<FieldId>(fieldId)) {
        case FieldId::FileType:
            track->filetype = utf16beToQString(data, fieldSize);
            break;
        case FieldId::FilePath:
            track->location = utf16beToQString(data, fieldSize);
            break;
        case FieldId::SongTitle:
            track->title = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Artist:
            track->artist = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Album:
            track->album = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Genre:
            track->genre = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Length: {
            bool ok;
            int duration = utf16beToQString(data, fieldSize).toInt(&ok);
            if (ok) {
                track->duration = duration;
            }
            break;
        }
        case FieldId::Bitrate:
            track->bitrate = utf16beToQString(data, fieldSize);
            break;
        case FieldId::SampleRate:
            track->samplerate = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Bpm: {
            bool ok;
            double bpm = utf16beToQString(data, fieldSize).toDouble(&ok);
            if (ok) {
                track->bpm = bpm;
            }
            break;
        }
        case FieldId::Comment:
            track->comment = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Grouping:
            track->grouping = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Label:
            track->label = utf16beToQString(data, fieldSize);
            break;
        case FieldId::Year: {
            // 4-digit year as string (YYYY)
            bool ok;
            int year = utf16beToQString(data, fieldSize).toInt(&ok);
            if (ok) {
                track->year = year;
            }
            break;
        }
        case FieldId::Key:
            track->key = utf16beToQString(data, fieldSize);
            break;
        case FieldId::BeatgridLocked:
            track->beatgridlocked = bytesToBoolean(data);
            break;
        case FieldId::Missing:
            if (fieldSize == 1) {
                track->missing = bytesToBoolean(data);
            }
            break;
        case FieldId::FileTime:
            // POSIX timestamp
            if (fieldSize == sizeof(quint32)) {
                track->filetime = bytesToUInt32(data);
            }
            break;
        case FieldId::DateAdded:
            // POSIX timestamp
            if (fieldSize == sizeof(quint32)) {
                track->datetimeadded = bytesToUInt32(data);
            }
            break;
        case FieldId::DateAddedText:
            // Ignore this field, but do not print a debug message. It's the
            // same as the regular DateAdded field, but this time the timestamp
            // is a string instead of an unsigned integer. Since we already
            // parse the integer version, it doesn't make sense to parse this.
            break;
        default: {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes).";
        }
        }

        headerData = buffer->read(kHeaderSize);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of track definition.";
        return false;
    }

    // Ignore tracks with empty location fields. The track location is used as
    // identifier by Serato (e.g. it's also used to reference them in Crates).
    if (track->location.isEmpty()) {
        qWarning() << "Found track with empty location field.";
        return false;
    }

    return true;
}

inline QString parseCrateTrackPath(QIODevice* buffer) {
    QString location;
    QByteArray headerData = buffer->read(kHeaderSize);
    while (headerData.length() == kHeaderSize) {
        quint32 fieldId = bytesToUInt32(headerData.mid(0, sizeof(quint32)));
        quint32 fieldSize = bytesToUInt32(headerData.mid(sizeof(quint32), kHeaderSize));

        // Read field data
        QByteArray data = buffer->read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qWarning() << "Failed to read "
                       << fieldSize
                       << " bytes for "
                       << fieldName
                       << " field.";
            return QString();
        }

        // Parse field data
        switch (static_cast<FieldId>(fieldId)) {
        case FieldId::TrackPath:
            location = utf16beToQString(data, fieldSize);
            break;
        default: {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes).";
        }
        }

        headerData = buffer->read(kHeaderSize);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of track definition.";
        return QString();
    }

    return location;
}

QString parseCrate(
        const QSqlDatabase& database,
        const QString& databasePath,
        const QString& crateFilePath,
        const QMap<QString, int>& trackIdMap) {
    QString crateName = QFileInfo(crateFilePath).baseName();
    qDebug() << "Parsing crate"
             << crateName
             << "at" << crateFilePath;

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for Serato parser."
                   << database.lastError();
        return QString();
    }

    QFile crateFile(crateFilePath);
    if (!crateFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file "
                   << crateFilePath
                   << " for reading.";
        return QString();
    }

    int playlistId = createPlaylist(database, crateFilePath, databasePath);
    if (playlistId < 0) {
        qWarning() << "Failed to create library playlist for "
                   << crateFilePath;
        return QString();
    }

    int trackCount = 0;
    QByteArray headerData = crateFile.read(kHeaderSize);
    while (headerData.length() == kHeaderSize) {
        quint32 fieldId = bytesToUInt32(headerData.mid(0, sizeof(quint32)));
        quint32 fieldSize = bytesToUInt32(headerData.mid(sizeof(quint32), kHeaderSize));

        // Read field data
        QByteArray data = crateFile.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qWarning() << "Failed to read "
                       << fieldSize
                       << " bytes for "
                       << fieldName
                       << " field from "
                       << crateFilePath
                       << ".";
            return QString();
        }

        // Parse field data
        switch (static_cast<FieldId>(fieldId)) {
        case FieldId::Version: {
            QString version = utf16beToQString(data, fieldSize);
            qDebug() << "Serato Database Version: "
                     << version;
            break;
        }
        case FieldId::Track: {
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QString location = parseCrateTrackPath(&buffer);
            if (!location.isEmpty()) {
                int trackId = trackIdMap.value(location, -1);
                insertTrackIntoPlaylist(database, playlistId, trackId, trackCount);
                trackCount++;
                break;
            }
            break;
        }
        default: {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes) in database "
                     << crateFilePath
                     << ".";
        }
        }

        headerData = crateFile.read(kHeaderSize);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of Serato database file "
                   << crateFilePath
                   << ".";
    }

    return crateName;
}

QString parseDatabase(mixxx::DbConnectionPoolPtr dbConnectionPool, TreeItem* databaseItem) {
    QString databaseName = databaseItem->getLabel();
    QString databaseFilePath = databaseItem->getData().toList()[0].toString();
    QDir databaseDir = QFileInfo(databaseFilePath).dir();

    QDir databaseRootDir = QDir(databaseDir);
    databaseRootDir.cdUp();

#if defined(__WINDOWS__)
    // On Windows, all paths are relative to drive root of the database (e.g.
    // "C:\"). Qt doesn't seem to provide a way to find it for a specific path,
    // so we just call cdUp() until it stops working.
    while (databaseRootDir.cdUp()) {
        // Nothing to do here
    }
#else
    // If the file is on an external drive, the database path are relative to
    // its mountpoint, i.e. the parent directory of the _Serato_
    // directory. This means we can just use the path as-is.
    //
    // If the file is not on an external drive, the paths are all relative to
    // the file system's root directory ("/").
    //
    // Serato does not exist on Linux, if it did, it would probably just mirror
    // the way paths are handled on OSX.
    if (databaseRootDir.canonicalPath().startsWith(QDir::homePath())) {
        databaseRootDir = QDir::root();
    }
#endif

    qDebug() << "Parsing Serato database"
             << databaseName
             << "at" << databaseFilePath;

    if (!QFile(databaseFilePath).exists()) {
        qWarning() << "Serato database file not found: "
                   << databaseFilePath;
        return databaseFilePath;
    }

    // The pooler limits the lifetime all thread-local connections,
    // that should be closed immediately before exiting this function.
    const mixxx::DbConnectionPooler dbConnectionPooler(dbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(dbConnectionPool);

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qWarning() << "Failed to open database for Serato parser."
                   << database.lastError();
        return QString();
    }

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    ScopedTransaction transaction(database);

    QSqlQuery query(database);
    query.prepare(
            "INSERT INTO " +
            kSeratoLibraryTable + " (" +
            LIBRARYTABLE_TITLE + ", " +
            LIBRARYTABLE_ARTIST + ", " +
            LIBRARYTABLE_ALBUM + ", " +
            LIBRARYTABLE_GENRE + ", " +
            LIBRARYTABLE_COMMENT + ", " +
            LIBRARYTABLE_GROUPING + ", " +
            LIBRARYTABLE_YEAR + ", " +
            LIBRARYTABLE_DURATION + ", " +
            LIBRARYTABLE_BITRATE + ", " +
            LIBRARYTABLE_SAMPLERATE + ", " +
            LIBRARYTABLE_BPM + ", " +
            LIBRARYTABLE_KEY + ", " +
            LIBRARYTABLE_LOCATION + ", " +
            LIBRARYTABLE_BPM_LOCK + ", " +
            LIBRARYTABLE_DATETIMEADDED +
            ", "
            "label, "
            "serato_db"
            ") VALUES ("
            ":title, "
            ":artist, "
            ":album, "
            ":genre, "
            ":comment, "
            ":grouping, "
            ":year, "
            ":duration, "
            ":bitrate, "
            ":samplerate, "
            ":bpm, "
            ":key, "
            ":location, "
            ":bpm_lock, "
            ":datetime_added, "
            ":label, "
            ":serato_db"
            ")");

    QFile databaseFile(databaseFilePath);
    if (!databaseFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file "
                   << databaseFilePath
                   << " for reading.";
        return QString();
    }

    int playlistId = createPlaylist(database, databaseFilePath, databaseDir.path());
    if (playlistId < 0) {
        qWarning() << "Failed to create library playlist for "
                   << databaseFilePath;
        return QString();
    }

    int trackCount = 0;
    QMap<QString, int> trackIdMap;
    QByteArray headerData = databaseFile.read(kHeaderSize);
    while (headerData.length() == kHeaderSize) {
        quint32 fieldId = bytesToUInt32(headerData.mid(0, sizeof(quint32)));
        quint32 fieldSize = bytesToUInt32(headerData.mid(sizeof(quint32), kHeaderSize));

        // Read field data
        QByteArray data = databaseFile.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qWarning() << "Failed to read "
                       << fieldSize
                       << " bytes for "
                       << fieldName
                       << " field from "
                       << databaseFilePath
                       << ".";
            return QString();
        }

        // Parse field data
        switch (static_cast<FieldId>(fieldId)) {
        case FieldId::Version: {
            QString version = utf16beToQString(data, fieldSize);
            qDebug() << "Serato Database Version: "
                     << version;
            break;
        }
        case FieldId::Track: {
            serato_track_t track;
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            if (parseTrack(&track, &buffer)) {
                QString location = databaseRootDir.absoluteFilePath(track.location);
                query.bindValue(":title", track.title);
                query.bindValue(":artist", track.artist);
                query.bindValue(":album", track.album);
                query.bindValue(":genre", track.genre);
                query.bindValue(":comment", track.comment);
                query.bindValue(":grouping", track.grouping);
                query.bindValue(":year", track.year);
                query.bindValue(":duration", track.duration);
                query.bindValue(":bitrate", track.bitrate);
                query.bindValue(":samplerate", track.samplerate);
                query.bindValue(":bpm", track.bpm);
                query.bindValue(":key", track.key);
                query.bindValue(":location", location);
                query.bindValue(":bpm_lock", track.beatgridlocked);
                query.bindValue(":datetime_added", track.datetimeadded);
                query.bindValue(":label", track.label);
                query.bindValue(":serato_db", databaseDir.path());

                if (!query.exec()) {
                    LOG_FAILED_QUERY(query);
                } else {
                    int trackId = query.lastInsertId().toInt();
                    insertTrackIntoPlaylist(database, playlistId, trackId, trackCount);
                    trackIdMap.insert(track.location, trackId);
                    trackCount++;
                }
                break;
            }
            break;
        }
        default: {
            QString fieldName = QString(headerData.mid(0, sizeof(quint32)));
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes) in database "
                     << databaseFilePath
                     << ".";
        }
        }

        headerData = databaseFile.read(kHeaderSize);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of Serato database file "
                   << databaseFilePath
                   << ".";
    }

    // Parse Crates
    QDir crateDir = QDir(databaseDir);
    if (crateDir.cd(kCrateDirectory)) {
        QStringList filters;
        filters << kCrateFilter;
        foreach (const QString& entry, crateDir.entryList(filters)) {
            QString crateFilePath = crateDir.filePath(entry);
            QString crateName = parseCrate(
                    database,
                    databaseDir.path(),
                    crateFilePath,
                    trackIdMap);
            if (!crateName.isEmpty()) {
                QList<QVariant> data;
                data << QVariant(crateFilePath)
                     << QVariant(true);
                TreeItem* crateItem = databaseItem->appendChild(crateName, data);
                crateItem->setIcon(QIcon(":/images/library/ic_library_crates.svg"));
            }
        }
    } else {
        qWarning() << "Failed to open crate directory: "
                   << databaseDir.filePath(kCrateDirectory);
    }

    // TODO: Parse Smart Crates

    transaction.commit();

    return databaseFilePath;
}

// This function is executed in a separate thread other than the main thread
QList<TreeItem*> findSeratoDatabases() {
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    // Build a list of directories that could contain the _Serato_ directory
    QFileInfoList databaseLocations;
    foreach (const QString& musicDir, QStandardPaths::standardLocations(QStandardPaths::MusicLocation)) {
        databaseLocations.append(QFileInfo(musicDir));
    }
#if defined(__WINDOWS__)
    // Repopulate drive list
    // Using drive.filePath() instead of drive.canonicalPath() as it
    // freezes interface too much if there is a network share mounted
    // (drive letter assigned) but unavailable
    //
    // drive.canonicalPath() make a system call to the underlying filesystem
    // introducing delay if it is unreadable.
    // drive.filePath() doesn't make any access to the filesystem and consequently
    // shorten the delay
    databaseLocations.append(QDir::drives());
#elif defined(__LINUX__)
    // To get devices on Linux, we look for directories under /media and
    // /run/media/$USER.
    const QString userName = QString::fromLocal8Bit(qgetenv("USER"));

    // Add folders under /media to devices.
    QDir mediaDir = QDir(QStringLiteral("/media/"));
    databaseLocations.append(
            mediaDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot));

    // Add folders under /media/$USER to devices.
    if (mediaDir.cd(userName)) {
        databaseLocations.append(
                mediaDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot));
    }

    // Add folders under /run/media/$USER to devices.
    QDir runMediaDir = QDir(QStringLiteral("/run/media/"));
    if (runMediaDir.cd(userName)) {
        databaseLocations.append(
                runMediaDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot));
    }
#elif defined(__APPLE__)
    QDir volumesDir = QDir(QStringLiteral("/Volumes"));
    databaseLocations.append(
            volumesDir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot));
#endif

    QList<TreeItem*> foundDatabases;
    foreach (QFileInfo databaseLocation, databaseLocations) {
        QDir databaseDir = QDir(databaseLocation.filePath());
        if (!databaseDir.cd(kDatabaseDirectory)) {
            continue;
        }

        if (!databaseDir.exists(kDatabaseFilename)) {
            continue;
        }

        QString displayPath = databaseLocation.filePath();
        if (displayPath.endsWith("/")) {
            displayPath.chop(1);
        }


        QList<QVariant> data;
        data << QVariant(databaseDir.filePath(kDatabaseFilename))
             << QVariant(false);

        TreeItem* foundDatabase = new TreeItem(
                std::move(displayPath),
                QVariant(data));

        foundDatabases << foundDatabase;
    }

    return foundDatabases;
}

bool createLibraryTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Serato library table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    title TEXT,"
            "    artist TEXT,"
            "    album TEXT,"
            "    genre TEXT,"
            "    comment TEXT,"
            "    grouping TEXT,"
            "    year INTEGER,"
            "    duration INTEGER,"
            "    bitrate TEXT,"
            "    samplerate TEXT,"
            "    bpm FLOAT,"
            "    key TEXT,"
            "    location TEXT,"
            "    bpm_lock INTEGER,"
            "    datetime_added DEFAULT CURRENT_TIMESTAMP,"
            "    label TEXT,"
            "    composer TEXT,"
            "    filename TEXT,"
            "    filetype TEXT,"
            "    remixer TEXT,"
            "    size INTEGER,"
            "    tracknumber TEXT,"
            "    serato_db TEXT"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool createPlaylistsTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Serato playlists table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY,"
            "    name TEXT,"
            "    serato_db TEXT"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool createPlaylistTracksTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Serato playlist tracks table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    playlist_id INTEGER REFERENCES serato_playlists(id),"
            "    track_id INTEGER REFERENCES serato_library(id),"
            "    position INTEGER"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool dropTable(QSqlDatabase& database, QString tableName) {
    qDebug() << "Dropping Serato table: " << tableName;

    QSqlQuery query(database);
    query.prepare("DROP TABLE IF EXISTS " + tableName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

} // anonymous namespace

SeratoFeature::SeratoFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig),
          m_icon(":/images/library/ic_library_serato.svg") {
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << LIBRARYTABLE_TITLE
            << LIBRARYTABLE_ARTIST
            << LIBRARYTABLE_ALBUM
            << LIBRARYTABLE_GENRE
            << LIBRARYTABLE_COMMENT
            << LIBRARYTABLE_GROUPING
            << LIBRARYTABLE_YEAR
            << LIBRARYTABLE_DURATION
            << LIBRARYTABLE_BITRATE
            << LIBRARYTABLE_SAMPLERATE
            << LIBRARYTABLE_BPM
            << LIBRARYTABLE_KEY
            << LIBRARYTABLE_TRACKNUMBER
            << LIBRARYTABLE_LOCATION
            << LIBRARYTABLE_BPM_LOCK;

    QStringList searchColumns;
    searchColumns
            << LIBRARYTABLE_ARTIST
            << LIBRARYTABLE_TITLE
            << LIBRARYTABLE_ALBUM
            << LIBRARYTABLE_YEAR
            << LIBRARYTABLE_GENRE
            << LIBRARYTABLE_TRACKNUMBER
            << LIBRARYTABLE_LOCATION
            << LIBRARYTABLE_COMMENT
            << LIBRARYTABLE_DURATION
            << LIBRARYTABLE_BITRATE
            << LIBRARYTABLE_BPM
            << LIBRARYTABLE_KEY;

    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(m_pTrackCollection, kSeratoLibraryTable, LIBRARYTABLE_ID, columns, false));
    m_trackSource->setSearchColumns(searchColumns);
    m_pSeratoPlaylistModel = new SeratoPlaylistModel(this, pLibrary->trackCollections(), m_trackSource);

    m_title = tr("Serato");

    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    // Drop any leftover temporary Serato database tables if they exist
    dropTable(database, kSeratoPlaylistTracksTable);
    dropTable(database, kSeratoPlaylistsTable);
    dropTable(database, kSeratoLibraryTable);

    // Create new temporary Serato database tables
    createLibraryTable(database, kSeratoLibraryTable);
    createPlaylistsTable(database, kSeratoPlaylistsTable);
    createPlaylistTracksTable(database, kSeratoPlaylistTracksTable);
    transaction.commit();

    connect(&m_databasesFutureWatcher,
            &QFutureWatcher<QList<TreeItem*>>::finished,
            this,
            &SeratoFeature::onSeratoDatabasesFound);
    connect(&m_tracksFutureWatcher,
            &QFutureWatcher<QString>::finished,
            this,
            &SeratoFeature::onTracksFound);

    // initialize the model
    m_childModel.setRootItem(TreeItem::newRoot(this));
}

SeratoFeature::~SeratoFeature() {
    m_databasesFuture.waitForFinished();
    m_tracksFuture.waitForFinished();

    // Drop temporary Serato database tables on shutdown
    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    dropTable(database, kSeratoPlaylistTracksTable);
    dropTable(database, kSeratoPlaylistsTable);
    dropTable(database, kSeratoLibraryTable);
    transaction.commit();

    delete m_pSeratoPlaylistModel;
}

void SeratoFeature::bindLibraryWidget(WLibrary* libraryWidget,
        KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit, SIGNAL(anchorClicked(const QUrl)), this, SLOT(htmlLinkClicked(const QUrl)));
    libraryWidget->registerView("SERATOHOME", edit);
}

void SeratoFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
}

BaseSqlTableModel* SeratoFeature::getPlaylistModelForPlaylist(QString playlist) {
    SeratoPlaylistModel* model = new SeratoPlaylistModel(this, m_pLibrary->trackCollections(), m_trackSource);
    model->setPlaylist(playlist);
    return model;
}

QVariant SeratoFeature::title() {
    return m_title;
}

QIcon SeratoFeature::getIcon() {
    return m_icon;
}

bool SeratoFeature::isSupported() {
    return true;
}

TreeItemModel* SeratoFeature::getChildModel() {
    return &m_childModel;
}

QString SeratoFeature::formatRootViewHtml() const {
    QString title = tr("Serato");
    QString summary = tr("Reads the following from the Serato Music directory and removable devices:");
    QStringList items;

    items << tr("Tracks")
          << tr("Crates");

    QString html;
    QString refreshLink = tr("Check for Serato databases (refresh)");
    html.append(QString("<h2>%1</h2>").arg(title));
    html.append(QString("<p>%1</p>").arg(summary));
    html.append(QString("<ul>"));
    for (const auto& item : qAsConst(items)) {
        html.append(QString("<li>%1</li>").arg(item));
    }
    html.append(QString("</ul>"));

    //Colorize links in lighter blue, instead of QT default dark blue.
    //Links are still different from regular text, but readable on dark/light backgrounds.
    //https://bugs.launchpad.net/mixxx/+bug/1744816
    html.append(QString("<a style=\"color:#0496FF;\" href=\"refresh\">%1</a>")
                        .arg(refreshLink));
    return html;
}

void SeratoFeature::refreshLibraryModels() {
}

void SeratoFeature::activate() {
    qDebug() << "SeratoFeature::activate()";

    // Let a worker thread do the parsing
    m_databasesFuture = QtConcurrent::run(findSeratoDatabases);
    m_databasesFutureWatcher.setFuture(m_databasesFuture);
    m_title = tr("(loading) Serato");
    //calls a slot in the sidebar model such that 'Serato (isLoading)' is displayed.
    emit featureIsLoading(this, true);

    emit enableCoverArtDisplay(true);
    emit switchToView("SERATOHOME");
}

void SeratoFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid())
        return;

    //access underlying TreeItem object
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }

    // TreeItem list data holds 2 values in a QList:
    //
    //     1. Playlist Name/Path (QString)
    //     2. isPlaylist (boolean)
    //
    // If the second element is false, then the database does still have to be
    // parsed.
    QList<QVariant> data = item->getData().toList();
    VERIFY_OR_DEBUG_ASSERT(data.size() == 2) {
        return;
    }
    QString playlist = data[0].toString();
    bool isPlaylist = data[1].toBool();

    qDebug() << "SeratoFeature::activateChild " << item->getLabel();

    if (!isPlaylist) {
        // Let a worker thread do the parsing
        m_tracksFuture = QtConcurrent::run(parseDatabase, static_cast<Library*>(parent())->dbConnectionPool(), item);
        m_tracksFutureWatcher.setFuture(m_tracksFuture);

        // This device is now a playlist element, future activations should
        // treat is as such
        data[1] = QVariant(true);
        item->setData(QVariant(data));
    } else {
        qDebug() << "Activate Serato Playlist: " << playlist;
        m_pSeratoPlaylistModel->setPlaylist(playlist);
        emit showTrackModel(m_pSeratoPlaylistModel);
    }
}

void SeratoFeature::onSeratoDatabasesFound() {
    QList<TreeItem*> foundDatabases = m_databasesFuture.result();
    TreeItem* root = m_childModel.getRootItem();

    QSqlDatabase database = m_pTrackCollection->database();

    if (foundDatabases.size() == 0) {
        // No Serato databases found

        if (root->childRows() > 0) {
            // Devices have since been unmounted
            m_childModel.removeRows(0, root->childRows());
        }
    } else {
        for (int databaseIndex = 0; databaseIndex < root->childRows(); databaseIndex++) {
            TreeItem* child = root->child(databaseIndex);
            bool removeChild = true;

            for (int foundDatabaseIndex = 0; foundDatabaseIndex < foundDatabases.size(); foundDatabaseIndex++) {
                TreeItem* databaseFound = foundDatabases[foundDatabaseIndex];

                if (databaseFound->getLabel() == child->getLabel()) {
                    removeChild = false;
                    break;
                }
            }

            if (removeChild) {
                // Device has since been unmounted, cleanup DB

                m_childModel.removeRows(databaseIndex, 1);
            }
        }

        QList<TreeItem*> childrenToAdd;

        for (int foundDatabaseIndex = 0; foundDatabaseIndex < foundDatabases.size(); foundDatabaseIndex++) {
            TreeItem* databaseFound = foundDatabases[foundDatabaseIndex];
            bool addNewChild = true;

            for (int databaseIndex = 0; databaseIndex < root->childRows(); databaseIndex++) {
                TreeItem* child = root->child(databaseIndex);

                if (databaseFound->getLabel() == child->getLabel()) {
                    // This database already exists in the TreeModel, don't add or parse is again
                    addNewChild = false;
                }
            }

            if (addNewChild) {
                childrenToAdd << databaseFound;
            }
        }

        if (!childrenToAdd.empty()) {
            m_childModel.insertTreeItemRows(childrenToAdd, 0);
        }
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Serato");
    emit featureLoadingFinished(this);
}

void SeratoFeature::onTracksFound() {
    qDebug() << "onTracksFound";
    m_childModel.triggerRepaint();

    QString databasePlaylist = m_tracksFuture.result();

    qDebug() << "Show Serato Database Playlist: " << databasePlaylist;

    m_pSeratoPlaylistModel->setPlaylist(databasePlaylist);
    emit showTrackModel(m_pSeratoPlaylistModel);
}
