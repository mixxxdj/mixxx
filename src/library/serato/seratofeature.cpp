// seratofeature.cpp
// Created 2020-01-31 by Jan Holthuis

#include "library/serato/seratofeature.h"

#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>

#include "engine/engine.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "library/dao/trackschema.h"
#include "track/beatfactory.h"
#include "track/cue.h"
#include "track/keyfactory.h"
#include "util/color/color.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/file.h"
#include "util/sandbox.h"
#include "waveform/waveform.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

// Serato Database Field IDs
// The "magic" value is the short 4 byte ascii code intepreted as quint32, so
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

const QString kDatabaseDirectory = "_Serato_";
const QString kDatabaseFilename = "database V2";
const QString kCrateDirectory = "Subcrates";
const QString kCrateFilter = "*.crate";
const QString kSmartCrateDirectory = "Smart Crates";
const QString kSmartCrateFilter = "*.scrate";

const QString kSeratoLibraryTable = "serato_library";
const QString kSeratoPlaylistsTable = "serato_library";
const QString kSeratoPlaylistTracksTable = "serato_library";

int createPlaylist(QSqlDatabase& database, QString name, QString databasePath) {
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

int insertTrackIntoPlaylist(QSqlDatabase& database, int playlistId, int trackId, int position) {
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

inline QString parseText(const QByteArray& data, const quint32 size) {
    return QTextCodec::codecForName("UTF-16BE")->toUnicode(data, size);
}

inline bool parseBoolean(const QByteArray& data) {
    return data.at(0) != 0;
}

inline quint32 parseUInt32(const QByteArray& data) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    return qFromBigEndian<quint32>(data);
#else
    return qFromBigEndian<quint32>(
            reinterpret_cast<const uchar*>(data.constData()));
#endif
}

inline bool parseTrack(serato_track_t& track, QIODevice& buffer) {
    QByteArray headerData = buffer.read(8);
    while (headerData.length() == 8) {
        QString fieldName = QString(headerData.mid(0, 4));
        quint32 fieldId = parseUInt32(headerData.mid(0, 4));
        quint32 fieldSize = parseUInt32(headerData.mid(4, 8));

        // Read field data
        QByteArray data = buffer.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
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
            track.filetype = parseText(data, fieldSize);
            break;
        case FieldId::FilePath:
            track.location = parseText(data, fieldSize);
            break;
        case FieldId::SongTitle:
            track.title = parseText(data, fieldSize);
            break;
        case FieldId::Artist:
            track.artist = parseText(data, fieldSize);
            break;
        case FieldId::Album:
            track.album = parseText(data, fieldSize);
            break;
        case FieldId::Genre:
            track.genre = parseText(data, fieldSize);
            break;
        case FieldId::Length: {
            bool ok;
            int duration = parseText(data, fieldSize).toInt(&ok);
            if (ok) {
                track.duration = duration;
            }
            break;
        }
        case FieldId::Bitrate:
            track.bitrate = parseText(data, fieldSize);
            break;
        case FieldId::SampleRate:
            track.samplerate = parseText(data, fieldSize);
            break;
        case FieldId::Bpm: {
            bool ok;
            double bpm = parseText(data, fieldSize).toDouble(&ok);
            if (ok) {
                track.bpm = bpm;
            }
            break;
        }
        case FieldId::Comment:
            track.comment = parseText(data, fieldSize);
            break;
        case FieldId::Grouping:
            track.grouping = parseText(data, fieldSize);
            break;
        case FieldId::Label:
            track.label = parseText(data, fieldSize);
            break;
        case FieldId::Year: {
            bool ok;
            int year = parseText(data, fieldSize).toInt(&ok);
            if (ok) {
                track.year = year;
            }
            break;
        }
        case FieldId::Key:
            track.key = parseText(data, fieldSize);
            break;
        case FieldId::BeatgridLocked:
            track.beatgridlocked = parseBoolean(data);
            break;
        case FieldId::Missing:
            track.missing = parseBoolean(data);
            break;
        case FieldId::FileTime:
            track.filetime = parseUInt32(data);
            break;
        case FieldId::DateAdded:
            track.datetimeadded = parseUInt32(data);
            break;
        case FieldId::DateAddedText:
            // Ignore this field, but do not print a debug message
            break;
        default:
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes).";
        }

        headerData = buffer.read(8);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of track definition.";
        return false;
    }

    if (track.location.isEmpty()) {
        qWarning() << "Found track with empty location field.";
        return false;
    }

    return true;
}

inline QString parseCrateTrack(QIODevice& buffer) {
    QString location;
    QByteArray headerData = buffer.read(8);
    while (headerData.length() == 8) {
        QString fieldName = QString(headerData.mid(0, 4));
        quint32 fieldId = parseUInt32(headerData.mid(0, 4));
        quint32 fieldSize = parseUInt32(headerData.mid(4, 8));

        // Read field data
        QByteArray data = buffer.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
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
            location = parseText(data, fieldSize);
            break;
        default:
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes).";
        }

        headerData = buffer.read(8);
    }

    if (headerData.length() != 0) {
        qWarning() << "Found "
                   << headerData.length()
                   << " extra bytes at end of track definition.";
        return QString();
    }

    return location;
}

QString parseCrate(QSqlDatabase& database, QString databasePath, QString crateFilePath, const QMap<QString, int>& trackIdMap) {
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

    QFile crateFile = QFile(crateFilePath);
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
    QByteArray headerData = crateFile.read(8);
    while (headerData.length() == 8) {
        QString fieldName = QString(headerData.mid(0, 4));
        quint32 fieldId = parseUInt32(headerData.mid(0, 4));
        quint32 fieldSize = parseUInt32(headerData.mid(4, 8));

        // Read field data
        QByteArray data = crateFile.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
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
            QString version = parseText(data, fieldSize);
            qDebug() << "Serato Database Version: "
                     << version;
            break;
        }
        case FieldId::Track: {
            QBuffer buffer = QBuffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QString location = parseCrateTrack(buffer);
            if (!location.isEmpty()) {
                int trackId = trackIdMap.value(location, -1);
                insertTrackIntoPlaylist(database, playlistId, trackId, trackCount);
                trackCount++;
                break;
            }
            break;
        }
        default:
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes) in database "
                     << crateFilePath
                     << ".";
        }

        headerData = crateFile.read(8);
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
    QDir databaseDir = QDir(databaseItem->getData().toList()[0].toString());
    QString databaseFilePath = databaseDir.filePath(kDatabaseFilename);

    QDir databaseRootDir = QDir(databaseDir);
    databaseRootDir.cdUp();

#if defined(__WINDOWS__)
    // Find drive letter (paths are relative to drive root on Windows)
    while (databaseRootDir.cdUp()) {
        // Nothing to do here
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
            "INSERT INTO " + kSeratoLibraryTable + " ("
            + LIBRARYTABLE_TITLE + ", "
            + LIBRARYTABLE_ARTIST + ", "
            + LIBRARYTABLE_ALBUM + ", "
            + LIBRARYTABLE_GENRE + ", "
            + LIBRARYTABLE_COMMENT + ", "
            + LIBRARYTABLE_GROUPING + ", "
            + LIBRARYTABLE_YEAR + ", "
            + LIBRARYTABLE_DURATION + ", "
            + LIBRARYTABLE_BITRATE + ", "
            + LIBRARYTABLE_SAMPLERATE + ", "
            + LIBRARYTABLE_BPM + ", "
            + LIBRARYTABLE_KEY + ", "
            + LIBRARYTABLE_LOCATION + ", "
            + LIBRARYTABLE_BPM_LOCK + ", "
            + LIBRARYTABLE_DATETIMEADDED + ", "
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

    QFile databaseFile = QFile(databaseFilePath);
    if (!databaseFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file "
                   << databaseFilePath
                   << " for reading.";
        return QString();
    }

    int playlistId = createPlaylist(database, databaseDir.path(), databaseDir.path());
    if (playlistId < 0) {
        qWarning() << "Failed to create library playlist for "
                   << databaseFilePath;
        return QString();
    }

    int trackCount = 0;
    QMap<QString, int> trackIdMap;
    QByteArray headerData = databaseFile.read(8);
    while (headerData.length() == 8) {
        QString fieldName = QString(headerData.mid(0, 4));
        quint32 fieldId = parseUInt32(headerData.mid(0, 4));
        quint32 fieldSize = parseUInt32(headerData.mid(4, 8));

        // Read field data
        QByteArray data = databaseFile.read(fieldSize);
        if (static_cast<quint32>(data.length()) != fieldSize) {
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
            QString version = parseText(data, fieldSize);
            qDebug() << "Serato Database Version: "
                     << version;
            break;
        }
        case FieldId::Track: {
            serato_track_t track;
            QBuffer buffer = QBuffer(&data);
            buffer.open(QIODevice::ReadOnly);
            if (parseTrack(track, buffer)) {
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
        default:
            qDebug() << "Ignoring unknown field "
                     << fieldName
                     << " ("
                     << fieldSize
                     << " bytes) in database "
                     << databaseFilePath
                     << ".";
        }

        headerData = databaseFile.read(8);
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
        foreach(const QString& entry, crateDir.entryList(filters)) {
            QString crateFilePath = crateDir.filePath(entry);
            QString crateName = parseCrate(database, databaseDir.path(), crateFilePath, trackIdMap);
            if (!crateName.isEmpty()) {
                QList<QVariant> data;
                data << QVariant(crateFilePath)
                     << QVariant(true);
                databaseItem->appendChild(crateName, data);
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
QList<TreeItem*> findSeratoDatabases(SeratoFeature* seratoFeature) {
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    QList<TreeItem*> foundDatabases;

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

    // Add folders under /media to devices.
    databaseLocations += QDir(QStringLiteral("/media")).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /media/$USER to devices.
    QDir mediaUserDir(QStringLiteral("/media/") + QString::fromLocal8Bit(qgetenv("USER")));
    databaseLocations += mediaUserDir.entryInfoList(
            QDir::AllDirs | QDir::NoDotAndDotDot);

    // Add folders under /run/media/$USER to devices.
    QDir runMediaUserDir(QStringLiteral("/run/media/") + QString::fromLocal8Bit(qgetenv("USER")));
    databaseLocations += runMediaUserDir.entryInfoList(
            QDir::AllDirs | QDir::NoDotAndDotDot);
#elif defined(__APPLE__)
    databaseLocations.append(QDir(QStringLiteral("/Volumes")).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot));
#endif

    foreach (QFileInfo databaseLocation, databaseLocations) {
        QDir databaseDir = QDir(databaseLocation.filePath());
        if (!databaseDir.cd(kDatabaseDirectory)) {
            continue;
        }

        if (!databaseDir.exists(kDatabaseFilename)) {
            continue;
        }

        TreeItem* foundDatabase = new TreeItem(seratoFeature);

        QString displayPath = databaseLocation.filePath();
        if (displayPath.endsWith("/")) {
            displayPath.chop(1);
        }

        foundDatabase->setLabel(displayPath);

        QList<QVariant> data;
        data << QVariant(databaseDir.path())
             << QVariant(false);
        foundDatabase->setData(data);

        foundDatabases << foundDatabase;
    }

    return foundDatabases;
}

void clearTable(QSqlDatabase& database, QString tableName) {
    QSqlQuery query(database);
    query.prepare("DELETE FROM " + tableName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "tableName:" << tableName;
        return;
    }

    query.prepare("DELETE FROM sqlite_sequence WHERE name=:name");
    query.bindValue(":name", tableName);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query) << "tableName:" << tableName;
        return;
    }

    qDebug() << "Serato table entries of '" << tableName << "' have been cleared.";
}

} // anonymous namespace

SeratoPlaylistModel::SeratoPlaylistModel(QObject* parent,
        TrackCollectionManager* trackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(parent, trackCollectionManager, "mixxx.db.model.serato.playlistmodel", "serato_playlists", "serato_playlist_tracks", trackSource) {
}

void SeratoPlaylistModel::initSortColumnMapping() {
    // Add a bijective mapping between the SortColumnIds and column indices
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }

    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ARTIST] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ARTIST);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TITLE] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TITLE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUM] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUM);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUMARTIST] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_YEAR] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_YEAR);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GENRE] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GENRE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMPOSER] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GROUPING] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_GROUPING);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TRACKNUMBER] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILETYPE] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_FILETYPE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_NATIVELOCATION] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_NATIVELOCATION);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMMENT] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COMMENT);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_DURATION] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DURATION);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BITRATE] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BPM] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_REPLAYGAIN] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_REPLAYGAIN);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_DATETIMEADDED] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_DATETIMEADDED);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TIMESPLAYED] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_TIMESPLAYED);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_RATING] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_RATING);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_KEY] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_KEY);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_PREVIEW] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COVERART] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_POSITION] = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(m_columnIndexBySortColumnId[sortColumn], sortColumn);
    }
}

TrackPointer SeratoPlaylistModel::getTrack(const QModelIndex& index) const {
    qDebug() << "SeratoTrackModel::getTrack";

    TrackPointer track = BaseExternalPlaylistModel::getTrack(index);

    return track;
}

bool SeratoPlaylistModel::isColumnHiddenByDefault(int column) {
    if (
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BPM_LOCK) ||
            column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_ID)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

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
            << LIBRARYTABLE_BPM_LOCK
            << "label"
            << "serato_db";

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

    //Clear any previous Serato database entries if they exist
    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    clearTable(database, kSeratoPlaylistTracksTable);
    clearTable(database, kSeratoPlaylistsTable);
    clearTable(database, kSeratoLibraryTable);
    transaction.commit();

    connect(&m_databasesFutureWatcher, &QFutureWatcher<QList<TreeItem*>>::finished, this, &SeratoFeature::onSeratoDatabasesFound);
    connect(&m_tracksFutureWatcher, &QFutureWatcher<QString>::finished, this, &SeratoFeature::onTracksFound);

    // initialize the model
    m_childModel.setRootItem(std::make_unique<TreeItem>(this));
}

SeratoFeature::~SeratoFeature() {
    m_databasesFuture.waitForFinished();
    m_tracksFuture.waitForFinished();
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
    for (const auto& item : items) {
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
    m_databasesFuture = QtConcurrent::run(findSeratoDatabases, this);
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
        ScopedTransaction transaction(database);
        transaction.commit();

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
