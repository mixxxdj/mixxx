#include "library/banshee/bansheeplaylistmodel.h"

#include <QSqlQuery>
#include <QtDebug>

#include "library/banshee/bansheedbconnection.h"
#include "library/dao/playlistdao.h"
#include "library/dao/trackschema.h"
#include "library/queryutil.h"
#include "library/trackcollectionmanager.h"
#include "mixer/playermanager.h"
#include "moc_bansheeplaylistmodel.cpp"
#include "track/track.h"

#define BANSHEE_TABLE "banshee"
#define CLM_TRACK_ID "track_id"
#define CLM_VIEW_ORDER "position"
#define CLM_ARTIST "artist"
#define CLM_TITLE "title"
#define CLM_DURATION "duration"
#define CLM_URI "location"
#define CLM_ALBUM "album"
#define CLM_ALBUM_ARTIST "album_artist"
#define CLM_YEAR "year"
#define CLM_RATING "rating"
#define CLM_GENRE "genre"
#define CLM_GROUPING "grouping"
#define CLM_TRACKNUMBER "tracknumber"
#define CLM_DATEADDED "datetime_added"
#define CLM_BPM "bpm"
#define CLM_BITRATE "bitrate"
#define CLM_COMMENT "comment"
#define CLM_PLAYCOUNT "timesplayed"
#define CLM_COMPOSER "composer"
#define CLM_PREVIEW "preview"
#define CLM_CRATE "crate"

namespace {

QAtomicInt sTableNumber;

const QString kTrackId = QStringLiteral(CLM_TRACK_ID);
const QString kViewOrder = QStringLiteral(CLM_VIEW_ORDER);
const QString kArtist = QStringLiteral(CLM_ARTIST);
const QString kTitle = QStringLiteral(CLM_TITLE);
const QString kDuration = QStringLiteral(CLM_DURATION);
const QString kUri = QStringLiteral(CLM_URI);
const QString kAlbum = QStringLiteral(CLM_ALBUM);
const QString kAlbumArtist = QStringLiteral(CLM_ALBUM_ARTIST);
const QString kYear = QStringLiteral(CLM_YEAR);
const QString kRating = QStringLiteral(CLM_RATING);
const QString kGenre = QStringLiteral(CLM_GENRE);
const QString kGrouping = QStringLiteral(CLM_GROUPING);
const QString kTracknumber = QStringLiteral(CLM_TRACKNUMBER);
const QString kDateadded = QStringLiteral(CLM_DATEADDED);
const QString kBpm = QStringLiteral(CLM_BPM);
const QString kBitrate = QStringLiteral(CLM_BITRATE);
const QString kComment = QStringLiteral(CLM_COMMENT);
const QString kPlaycount = QStringLiteral(CLM_PLAYCOUNT);
const QString kComposer = QStringLiteral(CLM_COMPOSER);
const QString kPreview = QStringLiteral(CLM_PREVIEW);
const QString kCrate = QStringLiteral(CLM_CRATE);

} // namespace

BansheePlaylistModel::BansheePlaylistModel(QObject* pParent, TrackCollectionManager* pTrackCollectionManager, BansheeDbConnection* pConnection)
        : BaseSqlTableModel(pParent, pTrackCollectionManager, "mixxx.db.model.banshee_playlist"),
          m_pConnection(pConnection),
          m_playlistId(kInvalidPlaylistId) {
    m_tempTableName = BANSHEE_TABLE + QString::number(sTableNumber.fetchAndAddAcquire(1));
}

BansheePlaylistModel::~BansheePlaylistModel() {
    dropTempTable();
}

void BansheePlaylistModel::dropTempTable() {
    if (m_playlistId >= 0) {
        // Clear old playlist
        m_playlistId = kInvalidPlaylistId;
        QSqlQuery query(m_database);
        QString strQuery("DROP TABLE IF EXISTS %1");
        if (!query.exec(strQuery.arg(m_tempTableName))) {
            LOG_FAILED_QUERY(query);
        }
    }
}

void BansheePlaylistModel::selectPlaylist(int playlistId) {
    // qDebug() << "BansheePlaylistModel::selectPlaylist" << this << playlistId;
    if (m_playlistId == playlistId) {
        qDebug() << "Already focused on playlist " << playlistId;
        return;
    }

    dropTempTable();

    if (playlistId >= 0) {
        // setup new playlist
        m_playlistId = playlistId;

        QSqlQuery query(m_database);
        if (!query.exec(QStringLiteral(
                    "CREATE TEMP TABLE IF NOT EXISTS %1 (" //
                    CLM_TRACK_ID " INTEGER, "              //
                    CLM_VIEW_ORDER " INTEGER, "            //
                    CLM_ARTIST " TEXT, "                   //
                    CLM_TITLE " TEXT, "                    //
                    CLM_DURATION " INTEGER, "              //
                    CLM_URI " TEXT, "                      //
                    CLM_ALBUM " TEXT, "                    //
                    CLM_ALBUM_ARTIST " TEXT, "             //
                    CLM_YEAR " INTEGER, "                  //
                    CLM_RATING " INTEGER, "                //
                    CLM_GENRE " TEXT, "                    //
                    CLM_GROUPING " TEXT, "                 //
                    CLM_TRACKNUMBER " INTEGER, "           //
                    CLM_DATEADDED " INTEGER, "             //
                    CLM_BPM " INTEGER, "                   //
                    CLM_BITRATE " INTEGER, "               //
                    CLM_COMMENT " TEXT, "                  //
                    CLM_PLAYCOUNT " INTEGER, "             //
                    CLM_COMPOSER " TEXT, "                 //
                    CLM_PREVIEW " TEXT)")
                                .arg(m_tempTableName))) {
            LOG_FAILED_QUERY(query);
        }

        QStringList insertColumns;
        insertColumns
                << CLM_TRACK_ID
                << CLM_VIEW_ORDER
                << CLM_ARTIST
                << CLM_TITLE
                << CLM_DURATION
                << CLM_URI
                << CLM_ALBUM
                << CLM_ALBUM_ARTIST
                << CLM_YEAR
                << CLM_RATING
                << CLM_GENRE
                << CLM_GROUPING
                << CLM_TRACKNUMBER
                << CLM_DATEADDED
                << CLM_BPM
                << CLM_BITRATE
                << CLM_COMMENT
                << CLM_PLAYCOUNT
                << CLM_COMPOSER;
        query.prepare(
                QStringLiteral("INSERT INTO %1 (%2) VALUES (:%3)")
                        .arg(m_tempTableName, insertColumns.join(", "), insertColumns.join(", :")));

        QList<struct BansheeDbConnection::PlaylistEntry> list =
                m_pConnection->getPlaylistEntries(playlistId);

        if (!list.isEmpty()) {
            beginInsertRows(QModelIndex(), 0, list.size() - 1);

            foreach (struct BansheeDbConnection::PlaylistEntry entry, list) {
                query.bindValue(":" CLM_TRACK_ID, entry.trackId);
                // Note: entry.viewOrder is 0 for all tracks if they have
                // never been sorted by the user
                query.bindValue(":" CLM_VIEW_ORDER, entry.viewOrder + 1);
                query.bindValue(":" CLM_ARTIST, entry.pArtist->name);
                query.bindValue(":" CLM_TITLE, entry.pTrack->title);
                query.bindValue(":" CLM_DURATION, entry.pTrack->duration / 1000);
                query.bindValue(":" CLM_URI, entry.pTrack->uri);
                query.bindValue(":" CLM_ALBUM, entry.pAlbum->title);
                query.bindValue(":" CLM_ALBUM_ARTIST, entry.pAlbumArtist->name);
                query.bindValue(":" CLM_YEAR, entry.pTrack->year);
                query.bindValue(":" CLM_RATING, entry.pTrack->rating);
                query.bindValue(":" CLM_GENRE, entry.pTrack->genre);
                query.bindValue(":" CLM_GROUPING, entry.pTrack->grouping);
                query.bindValue(":" CLM_TRACKNUMBER, entry.pTrack->tracknumber);
                QDateTime timeAdded;
                timeAdded.setSecsSinceEpoch(entry.pTrack->dateadded);
                query.bindValue(":" CLM_DATEADDED, timeAdded.toString(Qt::ISODate));
                query.bindValue(":" CLM_BPM, entry.pTrack->bpm);
                query.bindValue(":" CLM_BITRATE, entry.pTrack->bitrate);
                query.bindValue(":" CLM_COMMENT, entry.pTrack->comment);
                query.bindValue(":" CLM_PLAYCOUNT, entry.pTrack->playcount);
                query.bindValue(":" CLM_COMPOSER, entry.pTrack->composer);

                if (!query.exec()) {
                    LOG_FAILED_QUERY(query);
                }
                // qDebug() << "-----" << entry.pTrack->title << query.executedQuery();
            }

            endInsertRows();
        }
    }

    const QString idColumn = kTrackId;

    QStringList tableColumns = {
            kTrackId,
            kViewOrder,
            kPreview};

    QStringList trackSourceColumns = {
            kTrackId,
            kArtist,
            kTitle,
            kDuration,
            kUri,
            kAlbum,
            kAlbumArtist,
            kYear,
            kRating,
            kGenre,
            kGrouping,
            kTracknumber,
            kDateadded,
            kBpm,
            kBitrate,
            kComment,
            kPlaycount,
            kComposer};
    QStringList searchColumns = {
            kArtist,
            kAlbum,
            kAlbumArtist,
            kUri,
            kGrouping,
            kComment,
            kTitle,
            kGenre};

    auto trackSource = QSharedPointer<BaseTrackCache>::create(
            m_pTrackCollectionManager->internalCollection(),
            m_tempTableName,
            idColumn,
            std::move(trackSourceColumns),
            std::move(searchColumns),
            false);

    setTable(m_tempTableName, idColumn, std::move(tableColumns), trackSource);
    setSearch("");
    setDefaultSort(fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION),
            Qt::AscendingOrder);
    setSort(defaultSortColumn(), defaultSortOrder());
}

TrackModel::Capabilities BansheePlaylistModel::getCapabilities() const {
    return Capability::AddToTrackSet |
            Capability::AddToAutoDJ |
            Capability::LoadToDeck |
            Capability::LoadToSampler;
}

Qt::ItemFlags BansheePlaylistModel::flags(const QModelIndex& index) const {
    return readOnlyFlags(index);
}

TrackId BansheePlaylistModel::doGetTrackId(const TrackPointer& pTrack) const {
    if (pTrack) {
        for (int row = 0; row < rowCount(); ++row) {
            const QUrl rowUrl(getFieldString(index(row, 0),
                    ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION));
            if (mixxx::FileInfo::fromQUrl(rowUrl) == pTrack->getFileInfo()) {
                return TrackId(getFieldVariant(index(row, 0), CLM_VIEW_ORDER));
            }
        }
    }
    return TrackId();
}

TrackPointer BansheePlaylistModel::getTrack(const QModelIndex& index) const {
    QString location = getTrackLocation(index);

    if (location.isEmpty()) {
        // Track is lost
        return TrackPointer();
    }

    bool track_already_in_library = false;
    TrackPointer pTrack = m_pTrackCollectionManager->getOrAddTrack(
            TrackRef::fromFilePath(location),
            &track_already_in_library);

    // If this track was not in the Mixxx library it is now added and will be
    // saved with the metadata from Banshee. If it was already in the library
    // then we do not touch it so that we do not over-write the user's metadata.
    if (pTrack && !track_already_in_library) {
        pTrack->setArtist(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_ARTIST));
        pTrack->setTitle(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_TITLE));
        pTrack->setDuration(
                getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_DURATION)
                        .toDouble());
        pTrack->setAlbum(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_ALBUM));
        pTrack->setAlbumArtist(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_ALBUMARTIST));
        pTrack->setYear(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_YEAR));
        updateTrackGenre(pTrack.get(),
                getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_GENRE));
        pTrack->setGrouping(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_GROUPING));
        pTrack->setRating(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_RATING).toInt());
        pTrack->setTrackNumber(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_TRACKNUMBER));
        double bpm = getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_BPM).toDouble();
        pTrack->trySetBpm(bpm);
        pTrack->setBitrate(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_BITRATE).toInt());
        pTrack->setComment(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_COMMENT));
        pTrack->setComposer(getFieldString(index, ColumnCache::COLUMN_LIBRARYTABLE_COMPOSER));
    }
    return pTrack;
}

TrackId BansheePlaylistModel::getTrackId(const QModelIndex& index) const {
    const auto track = getTrack(index);
    if (track) {
        return track->getId();
    } else {
        return TrackId();
    }
}

QUrl BansheePlaylistModel::getTrackUrl(const QModelIndex& index) const {
    if (!index.isValid()) {
        return {};
    }
    return QUrl(getFieldString(index, ColumnCache::COLUMN_TRACKLOCATIONSTABLE_LOCATION));
}

// Gets the on-disk location of the track at the given location.
QString BansheePlaylistModel::getTrackLocation(const QModelIndex& index) const {
    const QUrl url = getTrackUrl(index);
    if (!url.isValid()) {
        return {};
    }

    if (url.isLocalFile()) {
        const QString location = mixxx::FileInfo::fromQUrl(url).location();
        if (!location.isEmpty()) {
            return location;
        }
    }

    // Try to convert a smb path location = url.toLocalFile();
    QString temp_location = url.toString();
    if (temp_location.startsWith("smb://")) {
        // Hack for samba mounts works only on German GNOME Linux
        // smb://daniel-desktop/volume/Musik/Lastfm/Limp Bizkit/Chocolate Starfish And The Hot Dog Flavored Water/06 - Rollin' (Air Raid Vehicle).mp3"
        // TODO(xxx): use gio instead

        QString location = QDir::homePath() + "/.gvfs/";
        location += temp_location.section('/', 3, 3);
        location += " auf ";
        location += temp_location.section('/', 2, 2);
        location += "/";
        location += temp_location.section('/', 4);

        return location;
    }

    return {};
}

bool BansheePlaylistModel::isColumnInternal(int column) {
    return (column == fieldIndex(ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_TRACKID) ||
            (PlayerManager::numPreviewDecks() == 0 &&
                    column == fieldIndex(ColumnCache::COLUMN_LIBRARYTABLE_PREVIEW)));
}
