// rekordboxfeature.cpp
// Created 05/24/2019 by Evan Dekker

#include "library/rekordbox/rekordboxfeature.h"

#include <mp3guessenc.h>

#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QtDebug>

#include "engine/engine.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/queryutil.h"
#include "library/rekordbox/rekordbox_anlz.h"
#include "library/rekordbox/rekordbox_pdb.h"
#include "library/rekordbox/rekordboxconstants.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "track/beatmap.h"
#include "track/cue.h"
#include "track/keyfactory.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/db/dbconnectionpooled.h"
#include "util/db/dbconnectionpooler.h"
#include "util/file.h"
#include "util/sandbox.h"
#include "waveform/waveform.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

#define IS_RECORDBOX_DEVICE "::isRecordboxDevice::"
#define IS_NOT_RECORDBOX_DEVICE "::isNotRecordboxDevice::"

namespace {

const QString kRekordboxLibraryTable = QStringLiteral("rekordbox_library");
const QString kRekordboxPlaylistsTable = QStringLiteral("rekordbox_playlists");
const QString kRekordboxPlaylistTracksTable = QStringLiteral("rekordbox_playlist_tracks");

const QString kPdbPath = QStringLiteral("PIONEER/rekordbox/export.pdb");
const QString kPLaylistPathDelimiter = QStringLiteral("-->");

enum class IDForColor : uint8_t {
    Pink = 1,
    Red,
    Orange,
    Yellow,
    Green,
    Aqua,
    Blue,
    Purple
};

constexpr mixxx::RgbColor kColorForIDPink(0xF870F8);
constexpr mixxx::RgbColor kColorForIDRed(0xF870900);
constexpr mixxx::RgbColor kColorForIDOrange(0xF8A030);
constexpr mixxx::RgbColor kColorForIDYellow(0xF8E331);
constexpr mixxx::RgbColor kColorForIDGreen(0x1EE000);
constexpr mixxx::RgbColor kColorForIDAqua(0x16C0F8);
constexpr mixxx::RgbColor kColorForIDBlue(0x0150F8);
constexpr mixxx::RgbColor kColorForIDPurple(0x9808F8);
constexpr mixxx::RgbColor kColorForIDNoColor(0x0);

struct memory_cue_loop_t {
    double startPosition;
    double endPosition;
    QString comment;
    mixxx::RgbColor::optional_t color;
};

bool createLibraryTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Rekordbox library table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    rb_id INTEGER,"
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

bool createPlaylistsTable(QSqlDatabase& database, const QString& tableName) {
    qDebug() << "Creating Rekordbox playlists table: " << tableName;

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
    qDebug() << "Creating Rekordbox playlist tracks table: " << tableName;

    QSqlQuery query(database);
    query.prepare(
            "CREATE TABLE IF NOT EXISTS " + tableName +
            " ("
            "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "    playlist_id INTEGER REFERENCES rekordbox_playlists(id),"
            "    track_id INTEGER REFERENCES rekordbox_library(id),"
            "    position INTEGER"
            ");");

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

bool dropTable(QSqlDatabase& database, QString tableName) {
    qDebug() << "Dropping Rekordbox table: " << tableName;

    QSqlQuery query(database);
    query.prepare("DROP TABLE IF EXISTS " + tableName);

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return false;
    }

    return true;
}

// This function is executed in a separate thread other than the main thread
QList<TreeItem*> findRekordboxDevices() {
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    QList<TreeItem*> foundDevices;

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

        QFileInfo rbDBFileInfo(drive.filePath() + kPdbPath);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            QString displayPath = drive.filePath();
            if (displayPath.endsWith("/")) {
                displayPath.chop(1);
            }
            QList<QString> data;
            data << drive.filePath();
            data << IS_RECORDBOX_DEVICE;
            TreeItem* foundDevice = new TreeItem(
                    std::move(displayPath),
                    QVariant(data));
            foundDevices << foundDevice;
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
        QFileInfo rbDBFileInfo(device.filePath() + QStringLiteral("/") + kPdbPath);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            QList<QString> data;
            data << device.filePath();
            data << IS_RECORDBOX_DEVICE;
            TreeItem* foundDevice = new TreeItem(
                    device.fileName(),
                    QVariant(data));
            foundDevices << foundDevice;
        }
    }
#else // __APPLE__
    QFileInfoList devices = QDir(QStringLiteral("/Volumes")).entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

    foreach (QFileInfo device, devices) {
        QFileInfo rbDBFileInfo(device.filePath() + QStringLiteral("/") + kPdbPath);

        if (rbDBFileInfo.exists() && rbDBFileInfo.isFile()) {
            QList<QString> data;
            data << device.filePath();
            data << IS_RECORDBOX_DEVICE;
            auto* foundDevice = new TreeItem(
                    device.fileName(),
                    QVariant(data));
            foundDevices << foundDevice;
        }
    }
#endif

    return foundDevices;
}

template<typename Base, typename T>
inline bool instanceof (const T* ptr) {
    return dynamic_cast<const Base*>(ptr) != nullptr;
}

QString toUnicode(std::string toConvert) {
    return QTextCodec::codecForName("UTF-16BE")->toUnicode(QByteArray(toConvert.c_str(), toConvert.length()));
}

// Functions getText and parseDeviceDB are roughly based on the following Java file:
// https://github.com/Deep-Symmetry/crate-digger/commit/f09fa9fc097a2a428c43245ddd542ac1370c1adc
// getText is needed because the strings in the PDB file "have a variety of obscure representations".

QString getText(rekordbox_pdb_t::device_sql_string_t* deviceString) {
    QString text;

    if (instanceof <rekordbox_pdb_t::device_sql_short_ascii_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_short_ascii_t* shortAsciiString =
                static_cast<rekordbox_pdb_t::device_sql_short_ascii_t*>(deviceString->body());
        text = QString::fromStdString(shortAsciiString->text());
    } else if (instanceof <rekordbox_pdb_t::device_sql_long_ascii_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_long_ascii_t* longAsciiString =
                static_cast<rekordbox_pdb_t::device_sql_long_ascii_t*>(deviceString->body());
        text = QString::fromStdString(longAsciiString->text());
    } else if (instanceof <rekordbox_pdb_t::device_sql_long_utf16be_t>(deviceString->body())) {
        rekordbox_pdb_t::device_sql_long_utf16be_t* longUtf16beString =
                static_cast<rekordbox_pdb_t::device_sql_long_utf16be_t*>(deviceString->body());
        text = toUnicode(longUtf16beString->text());
    }

    // Some strings read from Rekordbox *.PDB files contain random null characters
    // which if not removed cause Mixxx to crash when attempting to read file paths
    return text.remove(QChar('\x0'));
}

int createDevicePlaylist(QSqlDatabase& database, QString devicePath) {
    int playlistID = -1;

    QSqlQuery queryInsertIntoDevicePlaylist(database);
    queryInsertIntoDevicePlaylist.prepare(
            "INSERT INTO " + kRekordboxPlaylistsTable +
            " (name) "
            "VALUES (:name)");

    queryInsertIntoDevicePlaylist.bindValue(":name", devicePath);

    if (!queryInsertIntoDevicePlaylist.exec()) {
        LOG_FAILED_QUERY(queryInsertIntoDevicePlaylist)
                << "devicePath: " << devicePath;
        return playlistID;
    }

    QSqlQuery idQuery(database);
    idQuery.prepare("select id from " + kRekordboxPlaylistsTable + " where name=:path");
    idQuery.bindValue(":path", devicePath);

    if (!idQuery.exec()) {
        LOG_FAILED_QUERY(idQuery)
                << "devicePath: " << devicePath;
        return playlistID;
    }

    while (idQuery.next()) {
        playlistID = idQuery.value(idQuery.record().indexOf("id")).toInt();
    }

    return playlistID;
}

mixxx::RgbColor colorFromID(int colorID) {
    switch (static_cast<IDForColor>(colorID)) {
    case IDForColor::Pink:
        return kColorForIDPink;
    case IDForColor::Red:
        return kColorForIDRed;
    case IDForColor::Orange:
        return kColorForIDOrange;
    case IDForColor::Yellow:
        return kColorForIDYellow;
    case IDForColor::Green:
        return kColorForIDGreen;
    case IDForColor::Aqua:
        return kColorForIDAqua;
    case IDForColor::Blue:
        return kColorForIDBlue;
    case IDForColor::Purple:
        return kColorForIDPurple;
    }
    return kColorForIDNoColor;
}

void insertTrack(
        QSqlDatabase& database,
        rekordbox_pdb_t::track_row_t* track,
        QSqlQuery& query,
        QSqlQuery& queryInsertIntoDevicePlaylistTracks,
        QMap<uint32_t, QString>& artistsMap,
        QMap<uint32_t, QString>& albumsMap,
        QMap<uint32_t, QString>& genresMap,
        QMap<uint32_t, QString>& keysMap,
        QString devicePath,
        QString device,
        int audioFilesCount) {
    int rbID = static_cast<int>(track->id());
    QString title = getText(track->title());
    QString artist = artistsMap[track->artist_id()];
    QString album = albumsMap[track->album_id()];
    QString year = QString::number(track->year());
    QString genre = genresMap[track->genre_id()];
    QString location = devicePath + getText(track->file_path());
    float bpm = static_cast<float>(track->tempo() / 100.0);
    int bitrate = static_cast<int>(track->bitrate());
    QString key = keysMap[track->key_id()];
    int playtime = static_cast<int>(track->duration());
    int rating = static_cast<int>(track->rating());
    QString comment = getText(track->comment());
    QString tracknumber = QString::number(track->track_number());
    QString anlzPath = devicePath + getText(track->analyze_path());

    query.bindValue(":rb_id", rbID);
    query.bindValue(":artist", artist);
    query.bindValue(":title", title);
    query.bindValue(":album", album);
    query.bindValue(":genre", genre);
    query.bindValue(":year", year);
    query.bindValue(":duration", playtime);
    query.bindValue(":location", location);
    query.bindValue(":rating", rating);
    query.bindValue(":comment", comment);
    query.bindValue(":tracknumber", tracknumber);
    query.bindValue(":key", key);
    query.bindValue(":bpm", bpm);
    query.bindValue(":bitrate", bitrate);
    query.bindValue(":analyze_path", anlzPath);
    query.bindValue(":device", device);
    query.bindValue(":color", mixxx::RgbColor::toQVariant(colorFromID(static_cast<int>(track->color_id()))));

    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    int trackID = -1;
    QSqlQuery finderQuery(database);
    finderQuery.prepare("select id from " + kRekordboxLibraryTable + " where rb_id=:rb_id and device=:device");
    finderQuery.bindValue(":rb_id", rbID);
    finderQuery.bindValue(":device", device);

    if (!finderQuery.exec()) {
        LOG_FAILED_QUERY(finderQuery)
                << "rbID:" << rbID;
    }

    if (finderQuery.next()) {
        trackID = finderQuery.value(finderQuery.record().indexOf("id")).toInt();
    }

    // Insert into device all tracks playlist
    queryInsertIntoDevicePlaylistTracks.bindValue(":track_id", trackID);
    queryInsertIntoDevicePlaylistTracks.bindValue(":position", audioFilesCount);

    if (!queryInsertIntoDevicePlaylistTracks.exec()) {
        LOG_FAILED_QUERY(queryInsertIntoDevicePlaylistTracks)
                << "trackID:" << trackID
                << "position:" << audioFilesCount;
    }
}

void buildPlaylistTree(
        QSqlDatabase& database,
        TreeItem* parent,
        uint32_t parentID,
        QMap<uint32_t, QString>& playlistNameMap,
        QMap<uint32_t, bool>& playlistIsFolderMap,
        QMap<uint32_t, QMap<uint32_t, uint32_t>>& playlistTreeMap,
        QMap<uint32_t, QMap<uint32_t, uint32_t>>& playlistTrackMap,
        QString playlistPath,
        QString device);

QString parseDeviceDB(mixxx::DbConnectionPoolPtr dbConnectionPool, TreeItem* deviceItem) {
    QString device = deviceItem->getLabel();
    QString devicePath = deviceItem->getData().toList()[0].toString();

    qDebug() << "parseDeviceDB device: " << device << " devicePath: " << devicePath;

    QString dbPath = devicePath + QStringLiteral("/") + kPdbPath;

    if (!QFile(dbPath).exists()) {
        return devicePath;
    }

    // The pooler limits the lifetime all thread-local connections,
    // that should be closed immediately before exiting this function.
    const mixxx::DbConnectionPooler dbConnectionPooler(dbConnectionPool);
    QSqlDatabase database = mixxx::DbConnectionPooled(dbConnectionPool);

    //Open the database connection in this thread.
    VERIFY_OR_DEBUG_ASSERT(database.isOpen()) {
        qDebug() << "Failed to open database for Rekordbox parser."
                 << database.lastError();
        return QString();
    }

    //Give thread a low priority
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowPriority);

    ScopedTransaction transaction(database);

    QSqlQuery query(database);
    query.prepare(
            "INSERT INTO " + kRekordboxLibraryTable +
            " (rb_id, artist, title, album, year,"
            "genre,comment,tracknumber,bpm, bitrate,duration, location,"
            "rating,key,analyze_path,device,color) VALUES (:rb_id, :artist, :title, :album, :year,:genre,"
            ":comment, :tracknumber,:bpm, :bitrate,:duration, :location,"
            ":rating,:key,:analyze_path,:device,:color)");

    int audioFilesCount = 0;

    // Create a playlist for all the tracks on a device
    int playlistID = createDevicePlaylist(database, devicePath);

    QSqlQuery queryInsertIntoDevicePlaylistTracks(database);
    queryInsertIntoDevicePlaylistTracks.prepare(
            "INSERT INTO " + kRekordboxPlaylistTracksTable +
            " (playlist_id, track_id, position) "
            "VALUES (:playlist_id, :track_id, :position)");

    queryInsertIntoDevicePlaylistTracks.bindValue(":playlist_id", playlistID);

    std::ifstream ifs(dbPath.toStdString(), std::ifstream::binary);
    kaitai::kstream ks(&ifs);

    rekordbox_pdb_t reckordboxDB = rekordbox_pdb_t(&ks);

    // There are other types of tables (eg. COLOR), these are the only ones we are
    // interested at the moment. Perhaps when/if
    // https://bugs.launchpad.net/mixxx/+bug/1100882
    // is completed, this can be revisted.
    // Attempt was made to also recover HISTORY
    // playlists (which are found on removable Rekordbox devices), however
    // they didn't appear to contain valid row_ref_t structures.
    const int totalTables = 8;

    rekordbox_pdb_t::page_type_t tableOrder[totalTables] = {
            rekordbox_pdb_t::PAGE_TYPE_KEYS,
            rekordbox_pdb_t::PAGE_TYPE_GENRES,
            rekordbox_pdb_t::PAGE_TYPE_ARTISTS,
            rekordbox_pdb_t::PAGE_TYPE_ALBUMS,
            rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_ENTRIES,
            rekordbox_pdb_t::PAGE_TYPE_TRACKS,
            rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_TREE,
            rekordbox_pdb_t::PAGE_TYPE_HISTORY};

    QMap<uint32_t, QString> keysMap;
    QMap<uint32_t, QString> genresMap;
    QMap<uint32_t, QString> artistsMap;
    QMap<uint32_t, QString> albumsMap;
    QMap<uint32_t, QString> playlistNameMap;
    QMap<uint32_t, bool> playlistIsFolderMap;
    QMap<uint32_t, QMap<uint32_t, uint32_t>> playlistTreeMap;
    QMap<uint32_t, QMap<uint32_t, uint32_t>> playlistTrackMap;

    bool folderOrPlaylistFound = false;

    for (int tableOrderIndex = 0; tableOrderIndex < totalTables; tableOrderIndex++) {
        for (
                std::vector<rekordbox_pdb_t::table_t*>::iterator table = reckordboxDB.tables()->begin();
                table != reckordboxDB.tables()->end();
                ++table) {
            if ((*table)->type() == tableOrder[tableOrderIndex]) {
                uint16_t lastIndex = (*table)->last_page()->index();
                rekordbox_pdb_t::page_ref_t* currentRef = (*table)->first_page();

                while (true) {
                    rekordbox_pdb_t::page_t* page = currentRef->body();

                    if (page->is_data_page()) {
                        for (
                                std::vector<rekordbox_pdb_t::row_group_t*>::iterator rowGroup = page->row_groups()->begin();
                                rowGroup != page->row_groups()->end();
                                ++rowGroup) {
                            for (
                                    std::vector<rekordbox_pdb_t::row_ref_t*>::iterator rowRef = (*rowGroup)->rows()->begin();
                                    rowRef != (*rowGroup)->rows()->end();
                                    ++rowRef) {
                                if ((*rowRef)->present()) {
                                    switch (tableOrder[tableOrderIndex]) {
                                    case rekordbox_pdb_t::PAGE_TYPE_KEYS: {
                                        // Key found, update map
                                        rekordbox_pdb_t::key_row_t* key =
                                                static_cast<rekordbox_pdb_t::key_row_t*>((*rowRef)->body());
                                        keysMap[key->id()] = getText(key->name());
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_GENRES: {
                                        // Genre found, update map
                                        rekordbox_pdb_t::genre_row_t* genre =
                                                static_cast<rekordbox_pdb_t::genre_row_t*>((*rowRef)->body());
                                        genresMap[genre->id()] = getText(genre->name());
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_ARTISTS: {
                                        // Artist found, update map
                                        rekordbox_pdb_t::artist_row_t* artist =
                                                static_cast<rekordbox_pdb_t::artist_row_t*>((*rowRef)->body());
                                        artistsMap[artist->id()] = getText(artist->name());
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_ALBUMS: {
                                        // Album found, update map
                                        rekordbox_pdb_t::album_row_t* album =
                                                static_cast<rekordbox_pdb_t::album_row_t*>((*rowRef)->body());
                                        albumsMap[album->id()] = getText(album->name());
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_ENTRIES: {
                                        // Playlist to track mapping found, update map
                                        rekordbox_pdb_t::playlist_entry_row_t* playlistEntry =
                                                static_cast<rekordbox_pdb_t::playlist_entry_row_t*>((*rowRef)->body());
                                        playlistTrackMap[playlistEntry->playlist_id()][playlistEntry->entry_index()] =
                                                playlistEntry->track_id();
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_TRACKS: {
                                        // Track found, insert into database
                                        insertTrack(
                                                database, static_cast<rekordbox_pdb_t::track_row_t*>((*rowRef)->body()), query, queryInsertIntoDevicePlaylistTracks, artistsMap, albumsMap, genresMap, keysMap, devicePath, device, audioFilesCount);

                                        audioFilesCount++;
                                    } break;
                                    case rekordbox_pdb_t::PAGE_TYPE_PLAYLIST_TREE: {
                                        // Playlist tree node found, update map
                                        rekordbox_pdb_t::playlist_tree_row_t* playlistTree =
                                                static_cast<rekordbox_pdb_t::playlist_tree_row_t*>((*rowRef)->body());

                                        playlistNameMap[playlistTree->id()] = getText(playlistTree->name());
                                        playlistIsFolderMap[playlistTree->id()] = playlistTree->is_folder();
                                        playlistTreeMap[playlistTree->parent_id()][playlistTree->sort_order()] = playlistTree->id();

                                        folderOrPlaylistFound = true;
                                    } break;
                                    default:
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (currentRef->index() == lastIndex) {
                        break;
                    } else {
                        currentRef = page->next_page();
                    }
                }
            }
        }
    }

    if (audioFilesCount > 0 || folderOrPlaylistFound) {
        // If we have found anything, recursively build playlist/folder TreeItem children
        // for the original device TreeItem
        buildPlaylistTree(database, deviceItem, 0, playlistNameMap, playlistIsFolderMap, playlistTreeMap, playlistTrackMap, devicePath, device);
    }

    qDebug() << "Found: " << audioFilesCount << " audio files in Rekordbox device " << device;

    transaction.commit();

    return devicePath;
}

void buildPlaylistTree(
        QSqlDatabase& database,
        TreeItem* parent,
        uint32_t parentID,
        QMap<uint32_t, QString>& playlistNameMap,
        QMap<uint32_t, bool>& playlistIsFolderMap,
        QMap<uint32_t, QMap<uint32_t, uint32_t>>& playlistTreeMap,
        QMap<uint32_t, QMap<uint32_t, uint32_t>>& playlistTrackMap,
        QString playlistPath,
        QString device) {
    for (uint32_t childIndex = 0; childIndex < (uint32_t)playlistTreeMap[parentID].size(); childIndex++) {
        uint32_t childID = playlistTreeMap[parentID][childIndex];
        QString playlistItemName = playlistNameMap[childID];

        QString currentPath = playlistPath + kPLaylistPathDelimiter + playlistItemName;

        QList<QString> data;

        data << currentPath;
        data << IS_NOT_RECORDBOX_DEVICE;

        TreeItem* child = parent->appendChild(playlistItemName, QVariant(data));

        // Create a playlist for this child
        QSqlQuery queryInsertIntoPlaylist(database);
        queryInsertIntoPlaylist.prepare(
                "INSERT INTO " + kRekordboxPlaylistsTable +
                " (name) "
                "VALUES (:name)");

        queryInsertIntoPlaylist.bindValue(":name", currentPath);

        if (!queryInsertIntoPlaylist.exec()) {
            LOG_FAILED_QUERY(queryInsertIntoPlaylist)
                    << "currentPath" << currentPath;
            return;
        }

        QSqlQuery idQuery(database);
        idQuery.prepare("select id from " + kRekordboxPlaylistsTable + " where name=:path");
        idQuery.bindValue(":path", currentPath);

        if (!idQuery.exec()) {
            LOG_FAILED_QUERY(idQuery)
                    << "currentPath" << currentPath;
            return;
        }

        int playlistID = -1;
        while (idQuery.next()) {
            playlistID = idQuery.value(idQuery.record().indexOf("id")).toInt();
        }

        QSqlQuery queryInsertIntoPlaylistTracks(database);
        queryInsertIntoPlaylistTracks.prepare(
                "INSERT INTO " + kRekordboxPlaylistTracksTable +
                " (playlist_id, track_id, position) "
                "VALUES (:playlist_id, :track_id, :position)");

        if (playlistTrackMap.count(childID)) {
            // Add playlist tracks for children
            for (uint32_t trackIndex = 1; trackIndex <= static_cast<uint32_t>(playlistTrackMap[childID].size()); trackIndex++) {
                uint32_t rbTrackID = playlistTrackMap[childID][trackIndex];

                int trackID = -1;
                QSqlQuery finderQuery(database);
                finderQuery.prepare("select id from " + kRekordboxLibraryTable + " where rb_id=:rb_id and device=:device");
                finderQuery.bindValue(":rb_id", rbTrackID);
                finderQuery.bindValue(":device", device);

                if (!finderQuery.exec()) {
                    LOG_FAILED_QUERY(finderQuery)
                            << "rbTrackID:" << rbTrackID
                            << "device:" << device;
                    return;
                }

                if (finderQuery.next()) {
                    trackID = finderQuery.value(finderQuery.record().indexOf("id")).toInt();
                }

                queryInsertIntoPlaylistTracks.bindValue(":playlist_id", playlistID);
                queryInsertIntoPlaylistTracks.bindValue(":track_id", trackID);
                queryInsertIntoPlaylistTracks.bindValue(":position", static_cast<int>(trackIndex));

                if (!queryInsertIntoPlaylistTracks.exec()) {
                    LOG_FAILED_QUERY(queryInsertIntoPlaylistTracks)
                            << "playlistID:" << playlistID
                            << "trackID:" << trackID
                            << "trackIndex:" << trackIndex;

                    return;
                }
            }
        }

        if (playlistIsFolderMap[childID]) {
            // If this child is a folder (playlists are only leaf nodes), build playlist tree for it
            buildPlaylistTree(database, child, childID, playlistNameMap, playlistIsFolderMap, playlistTreeMap, playlistTrackMap, currentPath, device);
        }
    }
}

void clearDeviceTables(QSqlDatabase& database, TreeItem* child) {
    ScopedTransaction transaction(database);

    int trackID = -1;
    int playlistID = -1;
    QSqlQuery tracksQuery(database);
    tracksQuery.prepare("select id from " + kRekordboxLibraryTable + " where device=:device");
    tracksQuery.bindValue(":device", child->getLabel());

    QSqlQuery deletePlaylistsQuery(database);
    deletePlaylistsQuery.prepare("delete from " + kRekordboxPlaylistsTable + " where id=:id");

    QSqlQuery deletePlaylistTracksQuery(database);
    deletePlaylistTracksQuery.prepare("delete from " + kRekordboxPlaylistTracksTable + " where playlist_id=:playlist_id");

    if (!tracksQuery.exec()) {
        LOG_FAILED_QUERY(tracksQuery)
                << "device:" << child->getLabel();
    }

    while (tracksQuery.next()) {
        trackID = tracksQuery.value(tracksQuery.record().indexOf("id")).toInt();

        QSqlQuery playlistTracksQuery(database);
        playlistTracksQuery.prepare("select playlist_id from " + kRekordboxPlaylistTracksTable + " where track_id=:track_id");
        playlistTracksQuery.bindValue(":track_id", trackID);

        if (!playlistTracksQuery.exec()) {
            LOG_FAILED_QUERY(playlistTracksQuery)
                    << "trackID:" << trackID;
        }

        while (playlistTracksQuery.next()) {
            playlistID = playlistTracksQuery.value(playlistTracksQuery.record().indexOf("playlist_id")).toInt();

            deletePlaylistsQuery.bindValue(":id", playlistID);

            if (!deletePlaylistsQuery.exec()) {
                LOG_FAILED_QUERY(deletePlaylistsQuery)
                        << "playlistID:" << playlistID;
            }

            deletePlaylistTracksQuery.bindValue(":playlist_id", playlistID);

            if (!deletePlaylistTracksQuery.exec()) {
                LOG_FAILED_QUERY(deletePlaylistTracksQuery)
                        << "playlistID:" << playlistID;
            }
        }
    }

    QSqlQuery deleteTracksQuery(database);
    deleteTracksQuery.prepare("delete from " + kRekordboxLibraryTable + " where device=:device");
    deleteTracksQuery.bindValue(":device", child->getLabel());

    if (!deleteTracksQuery.exec()) {
        LOG_FAILED_QUERY(deleteTracksQuery)
                << "device:" << child->getLabel();
    }

    transaction.commit();
}

void setHotCue(TrackPointer track, double startPosition, double endPosition, int id, QString label, mixxx::RgbColor::optional_t color) {
    CuePointer pCue;
    bool hotCueFound = false;

    for (const CuePointer& trackCue : track->getCuePoints()) {
        if (trackCue->getHotCue() == id) {
            pCue = trackCue;
            hotCueFound = true;
            break;
        }
    }

    if (!hotCueFound) {
        pCue = CuePointer(track->createAndAddCue());
    }

    pCue->setStartPosition(startPosition);
    if (endPosition == Cue::kNoPosition) {
        pCue->setType(mixxx::CueType::HotCue);
    } else {
        pCue->setType(mixxx::CueType::Loop);
        pCue->setEndPosition(endPosition);
    }
    pCue->setHotCue(id);
    pCue->setLabel(label);
    if (color) {
        pCue->setColor(*color);
    }
}

void readAnalyze(TrackPointer track, double sampleRate, int timingOffset, bool ignoreCues, QString anlzPath) {
    if (!QFile(anlzPath).exists()) {
        return;
    }

    qDebug() << "Rekordbox ANLZ path:" << anlzPath << " for: " << track->getTitle();

    std::ifstream ifs(anlzPath.toStdString(), std::ifstream::binary);
    kaitai::kstream ks(&ifs);

    rekordbox_anlz_t anlz = rekordbox_anlz_t(&ks);

    double sampleRateKhz = sampleRate / 1000.0;
    double samples = sampleRateKhz * mixxx::kEngineChannelCount;

    QList<memory_cue_loop_t> memoryCuesAndLoops;
    int lastHotCueIndex = 0;

    for (std::vector<rekordbox_anlz_t::tagged_section_t*>::iterator section = anlz.sections()->begin(); section != anlz.sections()->end(); ++section) {
        switch ((*section)->fourcc()) {
        case rekordbox_anlz_t::SECTION_TAGS_BEAT_GRID: {
            if (!ignoreCues) {
                break;
            }

            rekordbox_anlz_t::beat_grid_tag_t* beatGridTag = static_cast<rekordbox_anlz_t::beat_grid_tag_t*>((*section)->body());

            QVector<double> beats;

            for (std::vector<rekordbox_anlz_t::beat_grid_beat_t*>::iterator beat = beatGridTag->beats()->begin(); beat != beatGridTag->beats()->end(); ++beat) {
                int time = static_cast<int>((*beat)->time()) - timingOffset;
                // Ensure no offset times are less than 1
                if (time < 1) {
                    time = 1;
                }
                beats << (sampleRateKhz * static_cast<double>(time));
            }

            QHash<QString, QString> extraVersionInfo;

            auto pBeats = new mixxx::BeatMap(*track, static_cast<SINT>(sampleRate), beats);
            pBeats->setSubVersion(mixxx::rekordboxconstants::beatsSubversion);
            track->setBeats(mixxx::BeatsPointer(pBeats));
        } break;
        case rekordbox_anlz_t::SECTION_TAGS_CUES: {
            if (ignoreCues) {
                break;
            }

            rekordbox_anlz_t::cue_tag_t* cuesTag = static_cast<rekordbox_anlz_t::cue_tag_t*>((*section)->body());

            for (std::vector<rekordbox_anlz_t::cue_entry_t*>::iterator cueEntry = cuesTag->cues()->begin(); cueEntry != cuesTag->cues()->end(); ++cueEntry) {
                int time = static_cast<int>((*cueEntry)->time()) - timingOffset;
                // Ensure no offset times are less than 1
                if (time < 1) {
                    time = 1;
                }
                double position = samples * static_cast<double>(time);

                switch (cuesTag->type()) {
                case rekordbox_anlz_t::CUE_LIST_TYPE_MEMORY_CUES: {
                    switch ((*cueEntry)->type()) {
                    case rekordbox_anlz_t::CUE_ENTRY_TYPE_MEMORY_CUE: {
                        memory_cue_loop_t memoryCue;
                        memoryCue.startPosition = position;
                        memoryCue.endPosition = Cue::kNoPosition;
                        memoryCue.color = mixxx::RgbColor::nullopt();
                        memoryCuesAndLoops << memoryCue;
                    } break;
                    case rekordbox_anlz_t::CUE_ENTRY_TYPE_LOOP: {
                        int endTime = static_cast<int>((*cueEntry)->loop_time()) - timingOffset;
                        // Ensure no offset times are less than 1
                        if (endTime < 1) {
                            endTime = 1;
                        }

                        memory_cue_loop_t loop;
                        loop.startPosition = position;
                        loop.endPosition = samples * static_cast<double>(endTime);
                        loop.color = mixxx::RgbColor::nullopt();
                        memoryCuesAndLoops << loop;
                    } break;
                    }
                } break;
                case rekordbox_anlz_t::CUE_LIST_TYPE_HOT_CUES: {
                    int hotCueIndex = static_cast<int>((*cueEntry)->hot_cue() - 1);
                    if (hotCueIndex > lastHotCueIndex) {
                        lastHotCueIndex = hotCueIndex;
                    }
                    setHotCue(
                            track,
                            position,
                            Cue::kNoPosition,
                            hotCueIndex,
                            QString(),
                            mixxx::RgbColor::nullopt());
                } break;
                }
            }
        } break;
        case rekordbox_anlz_t::SECTION_TAGS_CUES_2: {
            if (ignoreCues) {
                break;
            }

            rekordbox_anlz_t::cue_extended_tag_t* cuesExtendedTag = static_cast<rekordbox_anlz_t::cue_extended_tag_t*>((*section)->body());

            for (std::vector<rekordbox_anlz_t::cue_extended_entry_t*>::iterator cueExtendedEntry = cuesExtendedTag->cues()->begin(); cueExtendedEntry != cuesExtendedTag->cues()->end(); ++cueExtendedEntry) {
                int time = static_cast<int>((*cueExtendedEntry)->time()) - timingOffset;
                // Ensure no offset times are less than 1
                if (time < 1) {
                    time = 1;
                }
                double position = samples * static_cast<double>(time);

                switch (cuesExtendedTag->type()) {
                case rekordbox_anlz_t::CUE_LIST_TYPE_MEMORY_CUES: {
                    switch ((*cueExtendedEntry)->type()) {
                    case rekordbox_anlz_t::CUE_ENTRY_TYPE_MEMORY_CUE: {
                        memory_cue_loop_t memoryCue;
                        memoryCue.startPosition = position;
                        memoryCue.endPosition = Cue::kNoPosition;
                        memoryCue.comment = toUnicode((*cueExtendedEntry)->comment());
                        memoryCue.color = colorFromID(static_cast<int>((*cueExtendedEntry)->color_id()));
                        memoryCuesAndLoops << memoryCue;
                    } break;
                    case rekordbox_anlz_t::CUE_ENTRY_TYPE_LOOP: {
                        int endTime = static_cast<int>((*cueExtendedEntry)->loop_time()) - timingOffset;
                        // Ensure no offset times are less than 1
                        if (endTime < 1) {
                            endTime = 1;
                        }

                        memory_cue_loop_t loop;
                        loop.startPosition = position;
                        loop.endPosition = samples * static_cast<double>(endTime);
                        loop.comment = toUnicode((*cueExtendedEntry)->comment());
                        loop.color = colorFromID(static_cast<int>((*cueExtendedEntry)->color_id()));
                        memoryCuesAndLoops << loop;
                    } break;
                    }
                } break;
                case rekordbox_anlz_t::CUE_LIST_TYPE_HOT_CUES: {
                    int hotCueIndex = static_cast<int>((*cueExtendedEntry)->hot_cue() - 1);
                    if (hotCueIndex > lastHotCueIndex) {
                        lastHotCueIndex = hotCueIndex;
                    }
                    setHotCue(track,
                            position,
                            Cue::kNoPosition,
                            hotCueIndex,
                            toUnicode((*cueExtendedEntry)->comment()),
                            mixxx::RgbColor(qRgb(
                                    static_cast<int>(
                                            (*cueExtendedEntry)->color_red()),
                                    static_cast<int>(
                                            (*cueExtendedEntry)->color_green()),
                                    static_cast<int>((*cueExtendedEntry)
                                                             ->color_blue()))));
                } break;
                }
            }
        } break;
        default:
            break;
        }
    }

    if (memoryCuesAndLoops.size() > 0) {
        std::sort(memoryCuesAndLoops.begin(), memoryCuesAndLoops.end(), [](const memory_cue_loop_t& a, const memory_cue_loop_t& b) -> bool {
            return a.startPosition < b.startPosition;
        });

        bool mainCueFound = false;

        // Add memory cues and loops
        for (int memoryCueOrLoopIndex = 0; memoryCueOrLoopIndex < memoryCuesAndLoops.size(); memoryCueOrLoopIndex++) {
            memory_cue_loop_t memoryCueOrLoop = memoryCuesAndLoops[memoryCueOrLoopIndex];

            if (!mainCueFound && memoryCueOrLoop.endPosition == Cue::kNoPosition) {
                // Set first chronological memory cue as Mixxx MainCue
                track->setCuePoint(CuePosition(memoryCueOrLoop.startPosition));
                CuePointer pMainCue = track->findCueByType(mixxx::CueType::MainCue);
                pMainCue->setLabel(memoryCueOrLoop.comment);
                pMainCue->setColor(*memoryCueOrLoop.color);
                mainCueFound = true;
            } else {
                // Mixxx v2.4 will feature multiple loops, so these saved here will be usable
                // For 2.3, Mixxx treats them as hotcues and the first one will be loaded as the single loop Mixxx supports
                lastHotCueIndex++;
                setHotCue(
                        track,
                        memoryCueOrLoop.startPosition,
                        memoryCueOrLoop.endPosition,
                        lastHotCueIndex,
                        memoryCueOrLoop.comment,
                        memoryCueOrLoop.color);
            }
        }
    }
}

} // anonymous namespace

RekordboxPlaylistModel::RekordboxPlaylistModel(QObject* parent,
        TrackCollectionManager* trackCollectionManager,
        QSharedPointer<BaseTrackCache> trackSource)
        : BaseExternalPlaylistModel(parent, trackCollectionManager, "mixxx.db.model.rekordbox.playlistmodel", kRekordboxPlaylistsTable, kRekordboxPlaylistTracksTable, trackSource) {
}

void RekordboxPlaylistModel::initSortColumnMapping() {
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
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COLOR] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COLOR);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COVERART] = fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_COVERART);
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_POSITION] = fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION);

    m_sortColumnIdByColumnIndex.clear();
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        m_sortColumnIdByColumnIndex.insert(m_columnIndexBySortColumnId[sortColumn], sortColumn);
    }
}

TrackPointer RekordboxPlaylistModel::getTrack(const QModelIndex& index) const {
    qDebug() << "RekordboxTrackModel::getTrack";

    TrackPointer track = BaseExternalPlaylistModel::getTrack(index);
    QString location = index.sibling(index.row(), fieldIndex("location")).data().toString();

    if (!QFile(location).exists()) {
        return track;
    }

    // The following code accounts for timing offsets required to
    // correctly align timing information (cue points, loops, beatgrids)
    // exported from Rekordbox. This is caused by different MP3
    // decoders treating MP3s encoded in a variety of different cases
    // differently. The mp3guessenc library is used to determine which
    // case the MP3 is classified in. See the following PR for more
    // detailed information:
    // https://github.com/mixxxdj/mixxx/pull/2119

    int timingOffset = 0;

    if (location.toLower().endsWith(".mp3")) {
        int timingShiftCase = mp3guessenc_timing_shift_case(location.toStdString().c_str());

        qDebug() << "Timing shift case:" << timingShiftCase << "for MP3 file:" << location;

        switch (timingShiftCase) {
#ifdef __COREAUDIO__
        case EXIT_CODE_CASE_A:
            timingOffset = 12;
            break;
        case EXIT_CODE_CASE_B:
            timingOffset = 13;
            break;
        case EXIT_CODE_CASE_C:
            timingOffset = 26;
            break;
        case EXIT_CODE_CASE_D:
            timingOffset = 50;
            break;
#elif defined(__MAD__)
        case EXIT_CODE_CASE_A:
        case EXIT_CODE_CASE_D:
            timingOffset = 26;
            break;
#elif defined(__FFMPEG__)
        case EXIT_CODE_CASE_D:
            timingOffset = 26;
            break;
#endif
        }
    }

#ifdef __COREAUDIO__
    if (location.toLower().endsWith(".m4a")) {
        timingOffset = 48;
    }
#endif

    double sampleRate = static_cast<double>(track->getSampleRate());

    QString anlzPath = index.sibling(index.row(), fieldIndex("analyze_path")).data().toString();
    QString anlzPathExt = anlzPath.left(anlzPath.length() - 3) + "EXT";

    if (QFile(anlzPathExt).exists()) {
        // Beatgrids appear to be only correct in legacy ANLZ file
        readAnalyze(track, sampleRate, timingOffset, true, anlzPath);
        readAnalyze(track, sampleRate, timingOffset, false, anlzPathExt);
    } else {
        readAnalyze(track, sampleRate, timingOffset, false, anlzPath);
    }

    // Assume that the key of the file the has been analyzed in Recordbox is correct
    // and prevent the AnalyzerKey from re-analyzing.
    track->setKeys(KeyFactory::makeBasicKeysFromText(index.sibling(index.row(), fieldIndex("key")).data().toString(), mixxx::track::io::key::USER));

    track->setColor(mixxx::RgbColor::fromQVariant(index.sibling(index.row(), fieldIndex("color")).data()));

    return track;
}

bool RekordboxPlaylistModel::isColumnHiddenByDefault(int column) {
    if (column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_BITRATE)) {
        return true;
    }
    return BaseSqlTableModel::isColumnHiddenByDefault(column);
}

bool RekordboxPlaylistModel::isColumnInternal(int column) {
    return column == fieldIndex(ColumnCache::COLUMN_REKORDBOX_ANALYZE_PATH) || BaseExternalPlaylistModel::isColumnInternal(column);
}

RekordboxFeature::RekordboxFeature(
        Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseExternalLibraryFeature(pLibrary, pConfig),
          m_icon(":/images/library/ic_library_rekordbox.svg") {
    QString tableName = kRekordboxLibraryTable;
    QString idColumn = LIBRARYTABLE_ID;
    QStringList columns;
    columns << LIBRARYTABLE_ID
            << LIBRARYTABLE_ARTIST
            << LIBRARYTABLE_TITLE
            << LIBRARYTABLE_ALBUM
            << LIBRARYTABLE_YEAR
            << LIBRARYTABLE_GENRE
            << LIBRARYTABLE_TRACKNUMBER
            << LIBRARYTABLE_LOCATION
            << LIBRARYTABLE_COMMENT
            << LIBRARYTABLE_RATING
            << LIBRARYTABLE_DURATION
            << LIBRARYTABLE_BITRATE
            << LIBRARYTABLE_BPM
            << LIBRARYTABLE_KEY
            << LIBRARYTABLE_COLOR
            << REKORDBOX_ANALYZE_PATH;
    m_trackSource = QSharedPointer<BaseTrackCache>(
            new BaseTrackCache(m_pTrackCollection, tableName, idColumn, columns, false));
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
    m_trackSource->setSearchColumns(searchColumns);

    m_pRekordboxPlaylistModel = new RekordboxPlaylistModel(this, pLibrary->trackCollections(), m_trackSource);

    m_title = tr("Rekordbox");

    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    // Drop any leftover temporary Rekordbox database tables if they exist
    dropTable(database, kRekordboxPlaylistTracksTable);
    dropTable(database, kRekordboxPlaylistsTable);
    dropTable(database, kRekordboxLibraryTable);

    // Create new temporary Rekordbox database tables
    createLibraryTable(database, kRekordboxLibraryTable);
    createPlaylistsTable(database, kRekordboxPlaylistsTable);
    createPlaylistTracksTable(database, kRekordboxPlaylistTracksTable);
    transaction.commit();

    connect(&m_devicesFutureWatcher, SIGNAL(finished()), this, SLOT(onRekordboxDevicesFound()));
    connect(&m_tracksFutureWatcher, SIGNAL(finished()), this, SLOT(onTracksFound()));
    // initialize the model
    m_childModel.setRootItem(TreeItem::newRoot(this));
}

RekordboxFeature::~RekordboxFeature() {
    m_devicesFuture.waitForFinished();
    m_tracksFuture.waitForFinished();

    // Drop temporary Rekordbox database tables on shutdown
    QSqlDatabase database = m_pTrackCollection->database();
    ScopedTransaction transaction(database);
    dropTable(database, kRekordboxPlaylistTracksTable);
    dropTable(database, kRekordboxPlaylistsTable);
    dropTable(database, kRekordboxLibraryTable);
    transaction.commit();

    delete m_pRekordboxPlaylistModel;
}

void RekordboxFeature::bindLibraryWidget(WLibrary* libraryWidget,
        KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit, SIGNAL(anchorClicked(const QUrl)), this, SLOT(htmlLinkClicked(const QUrl)));
    libraryWidget->registerView("REKORDBOXHOME", edit);
}

void RekordboxFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "refresh") {
        activate();
    } else {
        qDebug() << "Unknown link clicked" << link;
    }
}

BaseSqlTableModel* RekordboxFeature::getPlaylistModelForPlaylist(QString playlist) {
    RekordboxPlaylistModel* model = new RekordboxPlaylistModel(this, m_pLibrary->trackCollections(), m_trackSource);
    model->setPlaylist(playlist);
    return model;
}

QVariant RekordboxFeature::title() {
    return m_title;
}

QIcon RekordboxFeature::getIcon() {
    return m_icon;
}

bool RekordboxFeature::isSupported() {
    return true;
}

TreeItemModel* RekordboxFeature::getChildModel() {
    return &m_childModel;
}

QString RekordboxFeature::formatRootViewHtml() const {
    QString title = tr("Rekordbox");
    QString summary = tr(
            "Reads databases for Pioneer CDJ / XDJ players. <br/>"
            "The database has to be exported to a USB or SD device <br/>"
            "with a FAT or HFS file system using the Rekordbox Export mode. <br/>"
            "Not supported are databases that have been moved to an external device via <br/>"
            "Preferences > Advanced > Database management. <br/>"
            "<br/>"
            "The following data is read:");

    QStringList items;

    items
            << tr("Folders")
            << tr("Playlists")
            << tr("Beatgrids")
            << tr("Hot cues")
            << tr("Memory cues")
            << tr("Loops (only the first loop is currently usable in Mixxx)");

    QString html;
    QString refreshLink = tr("Check for attached Rekordbox USB / SD devices (refresh)");
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

void RekordboxFeature::refreshLibraryModels() {
}

void RekordboxFeature::activate() {
    qDebug() << "RekordboxFeature::activate()";

    // Let a worker thread do the XML parsing
    m_devicesFuture = QtConcurrent::run(findRekordboxDevices);
    m_devicesFutureWatcher.setFuture(m_devicesFuture);
    m_title = tr("(loading) Rekordbox");
    //calls a slot in the sidebar model such that 'Rekordbox (isLoading)' is displayed.
    emit featureIsLoading(this, true);

    emit enableCoverArtDisplay(true);
    emit switchToView("REKORDBOXHOME");
}

void RekordboxFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid())
        return;

    //access underlying TreeItem object
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (!(item && item->getData().isValid())) {
        return;
    }

    // TreeItem list data holds 2 values in a QList and have different meanings.
    // If the 2nd QList element IS_RECORDBOX_DEVICE, the 1st element is the
    // filesystem device path, and the parseDeviceDB concurrent thread to parse
    // the Rekcordbox database is initiated. If the 2nd element is
    // IS_NOT_RECORDBOX_DEVICE, the 1st element is the playlist path and it is
    // activated.
    QList<QVariant> data = item->getData().toList();
    QString playlist = data[0].toString();
    bool doParseDeviceDB = data[1].toString() == IS_RECORDBOX_DEVICE;

    qDebug() << "RekordboxFeature::activateChild " << item->getLabel()
             << " playlist: " << playlist << " doParseDeviceDB: " << doParseDeviceDB;

    if (doParseDeviceDB) {
        qDebug() << "Parse Rekordbox Device DB: " << playlist;

        // Let a worker thread do the XML parsing
        m_tracksFuture = QtConcurrent::run(parseDeviceDB, static_cast<Library*>(parent())->dbConnectionPool(), item);
        m_tracksFutureWatcher.setFuture(m_tracksFuture);

        // This device is now a playlist element, future activations should treat is
        // as such
        data[1] = QVariant(IS_NOT_RECORDBOX_DEVICE);
        item->setData(QVariant(data));
    } else {
        qDebug() << "Activate Rekordbox Playlist: " << playlist;
        m_pRekordboxPlaylistModel->setPlaylist(playlist);
        emit showTrackModel(m_pRekordboxPlaylistModel);
    }
}

void RekordboxFeature::onRekordboxDevicesFound() {
    QList<TreeItem*> foundDevices = m_devicesFuture.result();
    TreeItem* root = m_childModel.getRootItem();

    QSqlDatabase database = m_pTrackCollection->database();

    if (foundDevices.size() == 0) {
        // No Rekordbox devices found
        ScopedTransaction transaction(database);

        dropTable(database, kRekordboxPlaylistTracksTable);
        dropTable(database, kRekordboxPlaylistsTable);
        dropTable(database, kRekordboxLibraryTable);

        // Create new temporary Rekordbox database tables
        createLibraryTable(database, kRekordboxLibraryTable);
        createPlaylistsTable(database, kRekordboxPlaylistsTable);
        createPlaylistTracksTable(database, kRekordboxPlaylistTracksTable);

        transaction.commit();

        if (root->childRows() > 0) {
            // Devices have since been unmounted
            m_childModel.removeRows(0, root->childRows());
        }
    } else {
        for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
            TreeItem* child = root->child(deviceIndex);
            bool removeChild = true;

            for (int foundDeviceIndex = 0; foundDeviceIndex < foundDevices.size(); foundDeviceIndex++) {
                TreeItem* deviceFound = foundDevices[foundDeviceIndex];

                if (deviceFound->getLabel() == child->getLabel()) {
                    removeChild = false;
                    break;
                }
            }

            if (removeChild) {
                // Device has since been unmounted, cleanup DB
                clearDeviceTables(database, child);

                m_childModel.removeRows(deviceIndex, 1);
            }
        }

        QList<TreeItem*> childrenToAdd;

        for (int foundDeviceIndex = 0; foundDeviceIndex < foundDevices.size(); foundDeviceIndex++) {
            TreeItem* deviceFound = foundDevices[foundDeviceIndex];
            bool addNewChild = true;

            for (int deviceIndex = 0; deviceIndex < root->childRows(); deviceIndex++) {
                TreeItem* child = root->child(deviceIndex);

                if (deviceFound->getLabel() == child->getLabel()) {
                    // This device already exists in the TreeModel, don't add or parse is again
                    addNewChild = false;
                }
            }

            if (addNewChild) {
                childrenToAdd << deviceFound;
            }
        }

        if (!childrenToAdd.empty()) {
            m_childModel.insertTreeItemRows(childrenToAdd, 0);
        }
    }

    // calls a slot in the sidebarmodel such that 'isLoading' is removed from the feature title.
    m_title = tr("Rekordbox");
    emit featureLoadingFinished(this);
}

void RekordboxFeature::onTracksFound() {
    qDebug() << "onTracksFound";
    m_childModel.triggerRepaint();

    QString devicePlaylist = m_tracksFuture.result();

    qDebug() << "Show Rekordbox Device Playlist: " << devicePlaylist;

    m_pRekordboxPlaylistModel->setPlaylist(devicePlaylist);
    emit showTrackModel(m_pRekordboxPlaylistModel);
}
